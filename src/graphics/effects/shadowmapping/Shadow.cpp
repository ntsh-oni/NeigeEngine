#include "Shadow.h"
#include "../../../graphics/resources/RendererResources.h"

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

	ImageTools::createImage(&defaultShadow.image, 1, 1, 1, 1, VK_SAMPLE_COUNT_1_BIT, physicalDevice.depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &defaultShadow.memoryInfo);
	ImageTools::createImageView(&defaultShadow.imageView, defaultShadow.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	ImageTools::transitionLayout(defaultShadow.image, physicalDevice.depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 1, 1);

	opaqueGraphicsPipeline.vertexShaderPath = "../shaders/shadowmapping/shadow.vert";
	opaqueGraphicsPipeline.renderPass = &renderPass;
	opaqueGraphicsPipeline.viewport = &viewport;
	opaqueGraphicsPipeline.multiSample = false;
	opaqueGraphicsPipeline.init();

	maskGraphicsPipeline.vertexShaderPath = "../shaders/shadowmapping/shadow.vert";
	maskGraphicsPipeline.fragmentShaderPath = "../shaders/shadowmapping/shadowMask.frag";
	maskGraphicsPipeline.renderPass = &renderPass;
	maskGraphicsPipeline.viewport = &viewport;
	maskGraphicsPipeline.multiSample = false;
	maskGraphicsPipeline.backfaceCulling = false;
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
	for (Image& image : images) {
		image.destroy();
	}
	for (size_t i = 0; i < framebuffers.size(); i++) {
		framebuffers[i].destroy();
	}
}
