#include "FXAA.h"
#include "../../resources/ImageTools.h"
#include "../../../graphics/resources/RendererResources.h"
#include "../../../graphics/resources/Samplers.h"
#include "../../../graphics/resources/ShaderResources.h"

void FXAA::init(Viewport fullscreenViewport) {
	viewport.init(static_cast<uint32_t>(fullscreenViewport.viewport.width), static_cast<uint32_t>(fullscreenViewport.viewport.height));

	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, swapchain.surfaceFormat.format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });

		renderPass.init(attachments, dependencies);
	}

	graphicsPipeline.vertexShaderPath = "../src/graphics/shaders/general/fullscreenTriangle.vert";
	graphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/fxaa/fxaa.frag";
	graphicsPipeline.renderPass = &renderPass;
	graphicsPipeline.viewport = &viewport;
	graphicsPipeline.multiSample = false;
	graphicsPipeline.frontFaceCCW = false;
	graphicsPipeline.depthWrite = false;
	graphicsPipeline.init();

	createResources(fullscreenViewport);
}

void FXAA::destroy() {
	destroyResources();
	renderPass.destroy();
	graphicsPipeline.destroy();
}

void FXAA::createResources(Viewport fullscreenViewport) {
	viewport.init(static_cast<uint32_t>(fullscreenViewport.viewport.width), static_cast<uint32_t>(fullscreenViewport.viewport.height));

	// Framebuffers
	framebuffers.resize(swapchainSize);

	{
		std::vector<std::vector<VkImageView>> framebufferAttachments;
		framebufferAttachments.resize(swapchainSize);
		framebuffers.resize(swapchainSize);
		for (uint32_t i = 0; i < swapchainSize; i++) {
			framebufferAttachments[i].push_back(swapchain.imageViews[i]);
			framebuffers[i].init(&renderPass, framebufferAttachments[i], static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1);
		}
	}

	// Descriptor set
	{
		descriptorSet.init(&graphicsPipeline, 0);

		VkDescriptorImageInfo postInfo = {};
		postInfo.sampler = trilinearOffscreenSampler;
		postInfo.imageView = postProcessImage.imageView;
		postInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet postWriteDescriptorSet = {};
		postWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		postWriteDescriptorSet.pNext = nullptr;
		postWriteDescriptorSet.dstSet = descriptorSet.descriptorSet;
		postWriteDescriptorSet.dstBinding = 0;
		postWriteDescriptorSet.dstArrayElement = 0;
		postWriteDescriptorSet.descriptorCount = 1;
		postWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		postWriteDescriptorSet.pImageInfo = &postInfo;
		postWriteDescriptorSet.pBufferInfo = nullptr;
		postWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(postWriteDescriptorSet);

		descriptorSet.update(writesDescriptorSet);
	}
}

void FXAA::destroyResources() {
	for (Framebuffer& framebuffer : framebuffers) {
		framebuffer.destroy();
	}
	framebuffers.clear();
	framebuffers.shrink_to_fit();
}

void FXAA::draw(CommandBuffer* commandBuffer, uint32_t framebufferIndex) {
	renderPass.begin(commandBuffer, framebuffers[framebufferIndex].framebuffer, { static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height) });
	graphicsPipeline.bind(commandBuffer);
	descriptorSet.bind(commandBuffer, &graphicsPipeline, 0);

	vkCmdDraw(commandBuffer->commandBuffer, 3, 1, 0, 0);

	renderPass.end(commandBuffer);
}