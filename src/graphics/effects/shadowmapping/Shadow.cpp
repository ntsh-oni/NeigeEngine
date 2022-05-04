#include "Shadow.h"
#include "../../../graphics/resources/RendererResources.h"
#include "../../../graphics/resources/ShaderResources.h"

void Shadow::init() {
	viewport.init(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);

	buffers.resize(framesInFlight);
	for (Buffer& buffer : buffers) {
		BufferTools::createUniformBuffer(buffer.buffer, sizeof(ShadowUniformBufferObject), &buffer.memoryInfo);
	}

	std::vector<RenderPassAttachment> attachments;
	attachments.push_back(RenderPassAttachment(AttachmentType::DEPTH, physicalDevice.depthFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, ClearDepthValue(1.0f, 0)));

	std::vector<SubpassDependency> dependencies;
	dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
	dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });

	renderPass.init(attachments, dependencies);

	ImageTools::loadDepth(1.0f, &defaultShadow.image, physicalDevice.depthFormat, &defaultShadow.memoryInfo);
	ImageTools::createImageView(&defaultShadow.imageView, defaultShadow.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	directionalImages.push_back(defaultShadow);
	spotImages.push_back(defaultShadow);

	opaqueGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/shadowmapping/shadow.vert";
	opaqueGraphicsPipeline.renderPass = &renderPass;
	opaqueGraphicsPipeline.viewport = &viewport;
	opaqueGraphicsPipeline.multiSample = false;
	opaqueGraphicsPipeline.init();

	maskGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/shadowmapping/shadow.vert";
	maskGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/shadowmapping/shadowMask.frag";
	maskGraphicsPipeline.renderPass = &renderPass;
	maskGraphicsPipeline.viewport = &viewport;
	maskGraphicsPipeline.multiSample = false;
	maskGraphicsPipeline.frontFaceCCW = false;
	maskGraphicsPipeline.externalSets.push_back(1);
	maskGraphicsPipeline.externalSets.push_back(2);
	maskGraphicsPipeline.externalDescriptorSetLayouts.push_back(materialsDescriptorSetLayout);
	maskGraphicsPipeline.externalDescriptorSetLayouts.push_back(perDrawDescriptorSetLayout);
	maskGraphicsPipeline.init();
}

void Shadow::destroy() {
	renderPass.destroy();
	opaqueGraphicsPipeline.destroy();
	maskGraphicsPipeline.destroy();
	for (Buffer& buffer : buffers) {
		buffer.destroy();
	}
	defaultShadow.destroy();
	for (size_t i = 1; i < directionalImages.size(); i++) {
		directionalImages[i].destroy();
	}
	for (size_t i = 1; i < spotImages.size(); i++) {
		spotImages[i].destroy();
	}
	for (Framebuffer& framebuffer : directionalFramebuffers) {
		framebuffer.destroy();
	}
	for (Framebuffer& framebuffer : spotFramebuffers) {
		framebuffer.destroy();
	}
}
