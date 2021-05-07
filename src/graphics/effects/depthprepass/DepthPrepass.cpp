#include "DepthPrepass.h"
#include "../../../utils/resources/BufferTools.h"
#include "../../../utils/resources/ImageTools.h"
#include "../../../graphics/resources/RendererResources.h"
#include "../../../graphics/resources/ShaderResources.h"

void DepthPrepass::init(Viewport fullscreenViewport) {
	viewport.init(static_cast<uint32_t>(fullscreenViewport.viewport.width), static_cast<uint32_t>(fullscreenViewport.viewport.height));

	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::DEPTH, physicalDevice.depthFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, ClearDepthValue(1.0f, 0)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });

		renderPass.init(attachments, dependencies);
	}

	opaqueGraphicsPipeline.vertexShaderPath = "../shaders/depthprepass/depthPrepass.vert";
	opaqueGraphicsPipeline.renderPass = &renderPass;
	opaqueGraphicsPipeline.viewport = &viewport;
	opaqueGraphicsPipeline.multiSample = false;
	opaqueGraphicsPipeline.backfaceCulling = true;
	opaqueGraphicsPipeline.init();

	maskGraphicsPipeline.vertexShaderPath = "../shaders/depthprepass/depthPrepass.vert";
	maskGraphicsPipeline.fragmentShaderPath = "../shaders/depthprepass/depthPrepassMask.frag";
	maskGraphicsPipeline.renderPass = &renderPass;
	maskGraphicsPipeline.viewport = &viewport;
	maskGraphicsPipeline.multiSample = false;
	maskGraphicsPipeline.backfaceCulling = true;
	maskGraphicsPipeline.externalSets.push_back(1);
	maskGraphicsPipeline.externalSets.push_back(2);
	maskGraphicsPipeline.externalDescriptorSetLayouts.push_back(materialsDescriptorSetLayout);
	maskGraphicsPipeline.externalDescriptorSetLayouts.push_back(perDrawDescriptorSetLayout);
	maskGraphicsPipeline.init();

	createResources(fullscreenViewport);
}

void DepthPrepass::destroy() {
	destroyResources();
	renderPass.destroy();
	opaqueGraphicsPipeline.destroy();
	maskGraphicsPipeline.destroy();
}

void DepthPrepass::createResources(Viewport fullscreenViewport) {
	viewport.init(static_cast<uint32_t>(fullscreenViewport.viewport.width), static_cast<uint32_t>(fullscreenViewport.viewport.height));
	
	// Image
	ImageTools::createImage(&image.image, 1, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1, VK_SAMPLE_COUNT_1_BIT, physicalDevice.depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &image.memoryInfo);
	ImageTools::createImageView(&image.imageView, image.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	// Framebuffer
	{
		std::vector<VkImageView> framebufferAttachements;
		framebufferAttachements.push_back(image.imageView);
		framebuffer.init(&renderPass, framebufferAttachements, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1);
	}
}

void DepthPrepass::destroyResources() {
	image.destroy();
	framebuffer.destroy();
}