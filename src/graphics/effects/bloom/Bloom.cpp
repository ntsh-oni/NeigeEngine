#include "Bloom.h"
#include "../../../utils/resources/ImageTools.h"
#include "../../../graphics/resources/RendererResources.h"
#include "../../../graphics/resources/ShaderResources.h"

void Bloom::init(Viewport fullscreenViewport) {
	viewport.init(static_cast<uint32_t>(fullscreenViewport.viewport.width) / BLOOMDOWNSCALE, static_cast<uint32_t>(fullscreenViewport.viewport.height) / BLOOMDOWNSCALE);

	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });

		resizeRenderPass.init(attachments, dependencies);
	}

	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });

		blurRenderPass.init(attachments, dependencies);
	}

	resizeGraphicsPipeline.vertexShaderPath = "../shaders/fullscreenTriangle.vert";
	resizeGraphicsPipeline.fragmentShaderPath = "../shaders/passthrough.frag";
	resizeGraphicsPipeline.renderPass = &blurRenderPass;
	resizeGraphicsPipeline.viewport = &viewport;
	resizeGraphicsPipeline.multiSample = false;
	resizeGraphicsPipeline.backfaceCulling = false;
	resizeGraphicsPipeline.depthWrite = false;
	resizeGraphicsPipeline.init();

	blurGraphicsPipeline.vertexShaderPath = "../shaders/fullscreenTriangle.vert";
	blurGraphicsPipeline.fragmentShaderPath = "../shaders/bloomBlur.frag";
	blurGraphicsPipeline.renderPass = &blurRenderPass;
	blurGraphicsPipeline.viewport = &viewport;
	blurGraphicsPipeline.multiSample = false;
	blurGraphicsPipeline.backfaceCulling = false;
	blurGraphicsPipeline.depthWrite = false;
	blurGraphicsPipeline.init();

	createResources(fullscreenViewport);
}

void Bloom::destroy() {
	destroyResources();
	resizeRenderPass.destroy();
	blurRenderPass.destroy();
	resizeGraphicsPipeline.destroy();
	blurGraphicsPipeline.destroy();
}

void Bloom::createResources(Viewport fullscreenViewport) {
	viewport.init(static_cast<uint32_t>(fullscreenViewport.viewport.width) / BLOOMDOWNSCALE, static_cast<uint32_t>(fullscreenViewport.viewport.height) / BLOOMDOWNSCALE);

	// Images
	ImageTools::createImage(&thresholdImage.image, 1, static_cast<uint32_t>(fullscreenViewport.viewport.width), static_cast<uint32_t>(fullscreenViewport.viewport.height), 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &thresholdImage.memoryInfo);
	ImageTools::createImageView(&thresholdImage.imageView, thresholdImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
	ImageTools::createImageSampler(&thresholdImage.imageSampler, 1, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VK_COMPARE_OP_ALWAYS);

	ImageTools::createImage(&blurredImage.image, 1, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &blurredImage.memoryInfo);
	ImageTools::createImageView(&blurredImage.imageView, blurredImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
	ImageTools::createImageSampler(&blurredImage.imageSampler, 1, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VK_COMPARE_OP_ALWAYS);

	ImageTools::createImage(&bloomImage.image, 1, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &bloomImage.memoryInfo);
	ImageTools::createImageView(&bloomImage.imageView, bloomImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
	ImageTools::createImageSampler(&bloomImage.imageSampler, 1, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VK_COMPARE_OP_ALWAYS);

	// Framebuffers
	{
		std::vector<VkImageView> framebufferAttachements;
		framebufferAttachements.push_back(bloomImage.imageView);
		resizeFramebuffer.init(&resizeRenderPass, framebufferAttachements, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1);
	}

	{
		std::vector<VkImageView> framebufferAttachements;
		framebufferAttachements.push_back(blurredImage.imageView);
		blurFramebuffer.init(&blurRenderPass, framebufferAttachements, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1);
	}

	{
		std::vector<VkImageView> framebufferAttachements;
		framebufferAttachements.push_back(bloomImage.imageView);
		backBlurFramebuffer.init(&blurRenderPass, framebufferAttachements, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1);
	}

	// Descriptor sets
	{
		resizeDescriptorSet.init(&resizeGraphicsPipeline, 0);

		VkDescriptorImageInfo thresholdInfo = {};
		thresholdInfo.sampler = bloomImage.imageSampler;
		thresholdInfo.imageView = thresholdImage.imageView;
		thresholdInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet thresholdWriteDescriptorSet = {};
		thresholdWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		thresholdWriteDescriptorSet.pNext = nullptr;
		thresholdWriteDescriptorSet.dstSet = resizeDescriptorSet.descriptorSet;
		thresholdWriteDescriptorSet.dstBinding = 0;
		thresholdWriteDescriptorSet.dstArrayElement = 0;
		thresholdWriteDescriptorSet.descriptorCount = 1;
		thresholdWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		thresholdWriteDescriptorSet.pImageInfo = &thresholdInfo;
		thresholdWriteDescriptorSet.pBufferInfo = nullptr;
		thresholdWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(thresholdWriteDescriptorSet);

		resizeDescriptorSet.update(writesDescriptorSet);
	}

	{
		blurDescriptorSet.init(&blurGraphicsPipeline, 0);

		VkDescriptorImageInfo bloomInfo = {};
		bloomInfo.sampler = blurredImage.imageSampler;
		bloomInfo.imageView = bloomImage.imageView;
		bloomInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet bloomWriteDescriptorSet = {};
		bloomWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		bloomWriteDescriptorSet.pNext = nullptr;
		bloomWriteDescriptorSet.dstSet = blurDescriptorSet.descriptorSet;
		bloomWriteDescriptorSet.dstBinding = 0;
		bloomWriteDescriptorSet.dstArrayElement = 0;
		bloomWriteDescriptorSet.descriptorCount = 1;
		bloomWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bloomWriteDescriptorSet.pImageInfo = &bloomInfo;
		bloomWriteDescriptorSet.pBufferInfo = nullptr;
		bloomWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(bloomWriteDescriptorSet);

		blurDescriptorSet.update(writesDescriptorSet);
	}

	{
		backBlurDescriptorSet.init(&blurGraphicsPipeline, 0);

		VkDescriptorImageInfo blurInfo = {};
		blurInfo.sampler = blurredImage.imageSampler;
		blurInfo.imageView = blurredImage.imageView;
		blurInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet blurWriteDescriptorSet = {};
		blurWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		blurWriteDescriptorSet.pNext = nullptr;
		blurWriteDescriptorSet.dstSet = backBlurDescriptorSet.descriptorSet;
		blurWriteDescriptorSet.dstBinding = 0;
		blurWriteDescriptorSet.dstArrayElement = 0;
		blurWriteDescriptorSet.descriptorCount = 1;
		blurWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		blurWriteDescriptorSet.pImageInfo = &blurInfo;
		blurWriteDescriptorSet.pBufferInfo = nullptr;
		blurWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(blurWriteDescriptorSet);

		backBlurDescriptorSet.update(writesDescriptorSet);
	}
}

void Bloom::destroyResources() {
	thresholdImage.destroy();
	blurredImage.destroy();
	bloomImage.destroy();
	resizeFramebuffer.destroy();
	blurFramebuffer.destroy();
	backBlurFramebuffer.destroy();
}

void Bloom::draw(CommandBuffer* commandBuffer) {
	// Resize
	resizeRenderPass.begin(commandBuffer, resizeFramebuffer.framebuffer, { static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height) });
	resizeGraphicsPipeline.bind(commandBuffer);
	resizeDescriptorSet.bind(commandBuffer, 0);

	vkCmdDraw(commandBuffer->commandBuffer, 3, 1, 0, 0);

	resizeRenderPass.end(commandBuffer);

	// Blur
	horizontalBlur = 1;

	for (int i = 0; i < BLURQUANTITY * 2; i++) {
		if (horizontalBlur == 1) {
			blurRenderPass.begin(commandBuffer, blurFramebuffer.framebuffer, { static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height) });
			blurGraphicsPipeline.bind(commandBuffer);
			blurDescriptorSet.bind(commandBuffer, 0);
		}
		else {
			blurRenderPass.begin(commandBuffer, backBlurFramebuffer.framebuffer, { static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height) });
			blurGraphicsPipeline.bind(commandBuffer);
			backBlurDescriptorSet.bind(commandBuffer, 0);
		}
		blurGraphicsPipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int), &horizontalBlur);

		vkCmdDraw(commandBuffer->commandBuffer, 3, 1, 0, 0);

		blurRenderPass.end(commandBuffer);

		horizontalBlur = (horizontalBlur == 1) ? 0 : 1;
	}
}