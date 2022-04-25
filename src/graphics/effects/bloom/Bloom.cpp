#include "Bloom.h"
#include "../../resources/ImageTools.h"
#include "../../../graphics/resources/RendererResources.h"
#include "../../../graphics/resources/Samplers.h"
#include "../../../graphics/resources/ShaderResources.h"

void Bloom::init(int downscale, float threshold, bool bigKernel, Viewport fullscreenViewport) {
	bloomDownscale = std::max(downscale, 1);
	viewport.init(std::max(static_cast<uint32_t>(fullscreenViewport.viewport.width) / bloomDownscale, static_cast<uint32_t>(1)), std::max(static_cast<uint32_t>(fullscreenViewport.viewport.height) / bloomDownscale, static_cast<uint32_t>(1)));

	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

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

	resizeGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/general/fullscreenTriangle.vert";
	resizeGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/bloom/bloomDownscale.frag";
	resizeGraphicsPipeline.renderPass = &blurRenderPass;
	resizeGraphicsPipeline.viewport = &viewport;
	resizeGraphicsPipeline.multiSample = false;
	resizeGraphicsPipeline.frontFaceCCW = false;
	resizeGraphicsPipeline.depthWrite = false;
	resizeGraphicsPipeline.init();

	blurViewport = viewport;

	blurGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/general/fullscreenTriangle.vert";
	blurGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/bloom/bloomBlur.frag";
	blurGraphicsPipeline.renderPass = &blurRenderPass;
	blurGraphicsPipeline.viewport = &blurViewport;
	blurGraphicsPipeline.multiSample = false;
	blurGraphicsPipeline.frontFaceCCW = false;
	blurGraphicsPipeline.depthWrite = false;
	blurGraphicsPipeline.init();

	bloomThreshold = threshold;
	blurBigKernel = bigKernel;

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
	viewport.init(std::max(static_cast<uint32_t>(fullscreenViewport.viewport.width) / bloomDownscale, static_cast<uint32_t>(1)), std::max(static_cast<uint32_t>(fullscreenViewport.viewport.height) / bloomDownscale, static_cast<uint32_t>(1)));

	// Images
	mipLevels = std::min<uint32_t>(static_cast<uint32_t>(std::floor(std::log2(std::min(viewport.viewport.width, viewport.viewport.height)))) + 1, 6);

	mipWidths.resize(mipLevels);
	mipHeights.resize(mipLevels);
	for (uint32_t mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
		mipWidths[mipLevel] = static_cast<uint32_t>(viewport.viewport.width * std::pow(0.5f, mipLevel));
		mipHeights[mipLevel] = static_cast<uint32_t>(viewport.viewport.height * std::pow(0.5f, mipLevel));
	}

	ImageTools::createImage(&blurredImage.image, 1, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &blurredImage.memoryInfo);

	ImageTools::createImage(&bloomImage.image, 1, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &bloomImage.memoryInfo);
	ImageTools::createImageView(&bloomImage.imageView, bloomImage.image, 0, 1, 0, mipLevels, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
	ImageTools::createImageView(&bloomLod0ImageView, bloomImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

	// Framebuffers
	{
		std::vector<VkImageView> framebufferAttachements;
		framebufferAttachements.push_back(bloomLod0ImageView);
		resizeFramebuffer.init(&resizeRenderPass, framebufferAttachements, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1);
	}

	blurImageViews.resize(mipLevels);
	blurFramebuffers.resize(mipLevels);
	backBlurImageViews.resize(mipLevels);
	backBlurFramebuffers.resize(mipLevels);

	{
		for (uint32_t mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
			ImageTools::createImageView(&blurImageViews[mipLevel], blurredImage.image, 0, 1, mipLevel, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
			std::vector<VkImageView> framebufferAttachements;
			framebufferAttachements.push_back(blurImageViews[mipLevel]);
			blurFramebuffers[mipLevel].init(&blurRenderPass, framebufferAttachements, mipWidths[mipLevel], mipHeights[mipLevel], 1);
		}
	}

	{
		for (uint32_t mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
			ImageTools::createImageView(&backBlurImageViews[mipLevel], bloomImage.image, 0, 1, mipLevel, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
			std::vector<VkImageView> framebufferAttachements;
			framebufferAttachements.push_back(backBlurImageViews[mipLevel]);
			backBlurFramebuffers[mipLevel].init(&blurRenderPass, framebufferAttachements, mipWidths[mipLevel], mipHeights[mipLevel], 1);
		}
	}

	// Descriptor sets
	{
		resizeDescriptorSet.init(&resizeGraphicsPipeline, 0);

		VkDescriptorImageInfo thresholdInfo = {};
		thresholdInfo.sampler = nearestOffscreenSampler;
		thresholdInfo.imageView = sceneImage.imageView;
		thresholdInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		resizeDescriptorSet.writesDescriptorSet.clear();
		resizeDescriptorSet.writesDescriptorSet.shrink_to_fit();

		resizeDescriptorSet.addWriteCombinedImageSampler(0, 1, &thresholdInfo);

		resizeDescriptorSet.update();
	}

	blurDescriptorSets.resize(mipLevels);
	backBlurDescriptorSets.resize(mipLevels);

	for (uint32_t mipLevel = 0; mipLevel < mipLevels; mipLevel++) {

		{
			blurDescriptorSets[mipLevel].init(&blurGraphicsPipeline, 0);

			VkDescriptorImageInfo bloomInfo = {};
			bloomInfo.sampler = nearestOffscreenSampler;
			bloomInfo.imageView = mipLevel == 0 ? bloomLod0ImageView : backBlurImageViews[mipLevel - 1];
			bloomInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			blurDescriptorSets[mipLevel].writesDescriptorSet.clear();
			blurDescriptorSets[mipLevel].writesDescriptorSet.shrink_to_fit();

			blurDescriptorSets[mipLevel].addWriteCombinedImageSampler(0, 1, &bloomInfo);

			blurDescriptorSets[mipLevel].update();
		}

		{
			backBlurDescriptorSets[mipLevel].init(&blurGraphicsPipeline, 0);

			VkDescriptorImageInfo blurInfo = {};
			blurInfo.sampler = nearestOffscreenSampler;
			blurInfo.imageView = blurImageViews[mipLevel];
			blurInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			backBlurDescriptorSets[mipLevel].writesDescriptorSet.clear();
			backBlurDescriptorSets[mipLevel].writesDescriptorSet.shrink_to_fit();

			backBlurDescriptorSets[mipLevel].addWriteCombinedImageSampler(0, 1, &blurInfo);

			backBlurDescriptorSets[mipLevel].update();
		}
	}
}

void Bloom::destroyResources() {
	blurredImage.destroy();
	vkDestroyImageView(logicalDevice.device, bloomLod0ImageView, nullptr);
	bloomImage.destroy();
	resizeFramebuffer.destroy();
	for (Framebuffer& framebuffer : blurFramebuffers) {
		framebuffer.destroy();
	}
	blurFramebuffers.clear();
	blurFramebuffers.shrink_to_fit();
	for (VkImageView imageView : blurImageViews) {
		vkDestroyImageView(logicalDevice.device, imageView, nullptr);
	}
	blurImageViews.clear();
	blurImageViews.shrink_to_fit();
	for (Framebuffer& framebuffer : backBlurFramebuffers) {
		framebuffer.destroy();
	}
	backBlurFramebuffers.clear();
	backBlurFramebuffers.shrink_to_fit();
	for (VkImageView imageView : backBlurImageViews) {
		vkDestroyImageView(logicalDevice.device, imageView, nullptr);
	}
	backBlurImageViews.clear();
	backBlurImageViews.shrink_to_fit();
}

void Bloom::draw(CommandBuffer* commandBuffer) {
	// Resize
	resizeRenderPass.begin(commandBuffer, resizeFramebuffer.framebuffer, { static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height) });
	resizeGraphicsPipeline.bind(commandBuffer);
	resizeDescriptorSet.bind(commandBuffer, &resizeGraphicsPipeline, 0);
	resizeGraphicsPipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &bloomThreshold);

	vkCmdDraw(commandBuffer->commandBuffer, 3, 1, 0, 0);

	resizeRenderPass.end(commandBuffer);

	// Blur
	for (uint32_t mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
		blurViewport.init(mipWidths[mipLevel], mipHeights[mipLevel]);

		// Horizontal
		int pushConstants[2] = { 1, blurBigKernel };

		blurRenderPass.begin(commandBuffer, blurFramebuffers[mipLevel].framebuffer, { mipWidths[mipLevel], mipHeights[mipLevel] });
		blurGraphicsPipeline.bind(commandBuffer);
		blurDescriptorSets[mipLevel].bind(commandBuffer, &blurGraphicsPipeline, 0);

		blurGraphicsPipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 2 * sizeof(int), &pushConstants);

		vkCmdDraw(commandBuffer->commandBuffer, 3, 1, 0, 0);

		blurRenderPass.end(commandBuffer);

		// Vertical
		pushConstants[0] = 0;

		blurRenderPass.begin(commandBuffer, backBlurFramebuffers[mipLevel].framebuffer, { mipWidths[mipLevel], mipHeights[mipLevel] });
		blurGraphicsPipeline.bind(commandBuffer);
		backBlurDescriptorSets[mipLevel].bind(commandBuffer, &blurGraphicsPipeline, 0);

		blurGraphicsPipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 2 * sizeof(int), &pushConstants);

		vkCmdDraw(commandBuffer->commandBuffer, 3, 1, 0, 0);

		blurRenderPass.end(commandBuffer);
	}
}