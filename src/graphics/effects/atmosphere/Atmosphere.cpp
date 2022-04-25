#include "Atmosphere.h"
#include "../../resources/ImageTools.h"
#include "../../../graphics/resources/RendererResources.h"
#include "../../../graphics/resources/Samplers.h"
#include "../../../graphics/resources/ShaderResources.h"

void Atmosphere::init(Viewport fullscreenViewport) {
	float defaultValue[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ImageTools::loadColor(defaultValue, &dummyImage.image, VK_FORMAT_R8G8B8A8_UNORM, &dummyImage.mipmapLevels, &dummyImage.memoryInfo);
	ImageTools::createImageView(&dummyImage.imageView, dummyImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	ImageTools::transitionLayout(dummyImage.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1);

	// Transmittance LUT
	transmittanceViewport.init(TRANSMITTANCE_WIDTH, TRANSMITTANCE_HEIGHT);

	ImageTools::createImage(&transmittanceImage.image, 1, TRANSMITTANCE_WIDTH, TRANSMITTANCE_HEIGHT, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &transmittanceImage.memoryInfo);
	ImageTools::createImageView(&transmittanceImage.imageView, transmittanceImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });

		transmittanceRenderPass.init(attachments, dependencies);
	}

	transmittanceGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/general/fullscreenTriangle.vert";
	transmittanceGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/atmosphere/transmittance.frag";
	transmittanceGraphicsPipeline.renderPass = &transmittanceRenderPass;
	transmittanceGraphicsPipeline.viewport = &transmittanceViewport;
	transmittanceGraphicsPipeline.multiSample = false;
	transmittanceGraphicsPipeline.frontFaceCCW = false;
	transmittanceGraphicsPipeline.depthWrite = false;
	transmittanceGraphicsPipeline.init();

	{
		std::vector<VkImageView> framebufferAttachments;
		framebufferAttachments.push_back(transmittanceImage.imageView);
		transmittanceFramebuffer.init(&transmittanceRenderPass, framebufferAttachments, TRANSMITTANCE_WIDTH, TRANSMITTANCE_HEIGHT, 1);
	}

	{
		transmittanceDescriptorSet.init(&transmittanceGraphicsPipeline, 0);

		VkDescriptorImageInfo dummyInfo = {};
		dummyInfo.sampler = trilinearEdgeBlackSampler;
		dummyInfo.imageView = dummyImage.imageView;
		dummyInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		transmittanceDescriptorSet.writesDescriptorSet.clear();
		transmittanceDescriptorSet.writesDescriptorSet.shrink_to_fit();

		transmittanceDescriptorSet.addWriteCombinedImageSampler(0, 1, &dummyInfo);

		transmittanceDescriptorSet.addWriteCombinedImageSampler(1, 1, &dummyInfo);

		transmittanceDescriptorSet.update();
	}

	// Multi scattering
	ImageTools::createImage(&multiScatteringImage.image, 1, MULTI_SCATTERING_WIDTH, MULTI_SCATTERING_HEIGHT, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &multiScatteringImage.memoryInfo);
	ImageTools::createImageView(&multiScatteringImage.imageView, multiScatteringImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
	ImageTools::transitionLayout(multiScatteringImage.image, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);

	multiScatteringComputePipeline.computeShaderPath = "../src/graphics/shaders/atmosphere/multiScattering.comp";
	multiScatteringComputePipeline.init();

	{
		multiScatteringDescriptorSet.init(&multiScatteringComputePipeline, 0);

		VkDescriptorImageInfo transmittanceInfo = {};
		transmittanceInfo.sampler = trilinearEdgeBlackSampler;
		transmittanceInfo.imageView = transmittanceImage.imageView;
		transmittanceInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo dummyInfo = {};
		dummyInfo.sampler = trilinearEdgeBlackSampler;
		dummyInfo.imageView = dummyImage.imageView;
		dummyInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo multiScatteringInfo = {};
		multiScatteringInfo.sampler = VK_NULL_HANDLE;
		multiScatteringInfo.imageView = multiScatteringImage.imageView;
		multiScatteringInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		multiScatteringDescriptorSet.writesDescriptorSet.clear();
		multiScatteringDescriptorSet.writesDescriptorSet.shrink_to_fit();

		multiScatteringDescriptorSet.addWriteCombinedImageSampler(0, 1, &transmittanceInfo);

		multiScatteringDescriptorSet.addWriteCombinedImageSampler(1, 1, &dummyInfo);

		multiScatteringDescriptorSet.addWriteStorageImage(2, 1, &multiScatteringInfo);

		multiScatteringDescriptorSet.update();
	}
	
	// Sky view
	skyViewViewport.init(SKY_VIEW_WIDTH, SKY_VIEW_HEIGHT);

	ImageTools::createImage(&skyViewImage.image, 1, SKY_VIEW_WIDTH, SKY_VIEW_HEIGHT, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &skyViewImage.memoryInfo);
	ImageTools::createImageView(&skyViewImage.imageView, skyViewImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });

		skyViewRenderPass.init(attachments, dependencies);
	}

	skyViewGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/general/fullscreenTriangle.vert";
	skyViewGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/atmosphere/skyView.frag";
	skyViewGraphicsPipeline.renderPass = &skyViewRenderPass;
	skyViewGraphicsPipeline.viewport = &skyViewViewport;
	skyViewGraphicsPipeline.multiSample = false;
	skyViewGraphicsPipeline.frontFaceCCW = false;
	skyViewGraphicsPipeline.depthWrite = false;
	skyViewGraphicsPipeline.init();

	{
		std::vector<VkImageView> framebufferAttachments;
		framebufferAttachments.push_back(skyViewImage.imageView);
		skyViewFramebuffer.init(&skyViewRenderPass, framebufferAttachments, SKY_VIEW_WIDTH, SKY_VIEW_HEIGHT, 1);
	}

	skyViewDescriptorSets.resize(framesInFlight);
	for (uint32_t i = 0; i < framesInFlight; i++) {
		skyViewDescriptorSets[i].init(&skyViewGraphicsPipeline, 0);

		VkDescriptorImageInfo transmittanceInfo = {};
		transmittanceInfo.sampler = trilinearEdgeBlackSampler;
		transmittanceInfo.imageView = transmittanceImage.imageView;
		transmittanceInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo multiScatteringInfo = {};
		multiScatteringInfo.sampler = trilinearEdgeBlackSampler;
		multiScatteringInfo.imageView = multiScatteringImage.imageView;
		multiScatteringInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorBufferInfo cameraInfo = {};
		cameraInfo.buffer = cameraBuffers.at(i).buffer;
		cameraInfo.offset = 0;
		cameraInfo.range = sizeof(CameraUniformBufferObject);

		skyViewDescriptorSets[i].writesDescriptorSet.clear();
		skyViewDescriptorSets[i].writesDescriptorSet.shrink_to_fit();

		skyViewDescriptorSets[i].addWriteCombinedImageSampler(0, 1, &transmittanceInfo);

		skyViewDescriptorSets[i].addWriteCombinedImageSampler(1, 1, &multiScatteringInfo);

		skyViewDescriptorSets[i].addWriteUniformBuffer(2, 1, &cameraInfo);

		skyViewDescriptorSets[i].update();
	}

	// Camera volume
	cameraVolumeViewport.init(CAMERA_VOLUME_WIDTH, CAMERA_VOLUME_HEIGHT);

	ImageTools::create3DImage(&cameraVolumeImage.image, CAMERA_VOLUME_WIDTH, CAMERA_VOLUME_HEIGHT, 32, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &cameraVolumeImage.memoryInfo);
	ImageTools::createImageView(&cameraVolumeImage.imageView, cameraVolumeImage.image, 0, 32, 0, 1, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
	ImageTools::createImageView(&cameraVolumeToSampleImage.imageView, cameraVolumeImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_3D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });

		cameraVolumeRenderPass.init(attachments, dependencies);
	}

	cameraVolumeGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/general/fullscreenTriangleInstanced.vert";
	cameraVolumeGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/atmosphere/cameraVolume.frag";
	cameraVolumeGraphicsPipeline.renderPass = &cameraVolumeRenderPass;
	cameraVolumeGraphicsPipeline.viewport = &cameraVolumeViewport;
	cameraVolumeGraphicsPipeline.multiSample = false;
	cameraVolumeGraphicsPipeline.frontFaceCCW = false;
	cameraVolumeGraphicsPipeline.depthWrite = false;
	cameraVolumeGraphicsPipeline.init();

	{
		std::vector<VkImageView> framebufferAttachments;
		framebufferAttachments.push_back(cameraVolumeImage.imageView);
		cameraVolumeFramebuffer.init(&cameraVolumeRenderPass, framebufferAttachments, CAMERA_VOLUME_WIDTH, CAMERA_VOLUME_HEIGHT, 32);
	}

	cameraVolumeDescriptorSets.resize(framesInFlight);
	for (uint32_t i = 0; i < framesInFlight; i++) {
		cameraVolumeDescriptorSets[i].init(&cameraVolumeGraphicsPipeline, 0);

		VkDescriptorImageInfo transmittanceInfo = {};
		transmittanceInfo.sampler = trilinearEdgeBlackSampler;
		transmittanceInfo.imageView = transmittanceImage.imageView;
		transmittanceInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo multiScatteringInfo = {};
		multiScatteringInfo.sampler = trilinearEdgeBlackSampler;
		multiScatteringInfo.imageView = multiScatteringImage.imageView;
		multiScatteringInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorBufferInfo cameraInfo = {};
		cameraInfo.buffer = cameraBuffers.at(i).buffer;
		cameraInfo.offset = 0;
		cameraInfo.range = sizeof(CameraUniformBufferObject);

		cameraVolumeDescriptorSets[i].writesDescriptorSet.clear();
		cameraVolumeDescriptorSets[i].writesDescriptorSet.shrink_to_fit();

		cameraVolumeDescriptorSets[i].addWriteCombinedImageSampler(0, 1, &transmittanceInfo);

		cameraVolumeDescriptorSets[i].addWriteCombinedImageSampler(1, 1, &multiScatteringInfo);

		cameraVolumeDescriptorSets[i].addWriteUniformBuffer(2, 1, &cameraInfo);

		cameraVolumeDescriptorSets[i].update();
	}
	
	// Ray marching
	rayMarchingViewport.init(static_cast<uint32_t>(fullscreenViewport.viewport.width), static_cast<uint32_t>(fullscreenViewport.viewport.height));

	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });

		rayMarchingRenderPass.init(attachments, dependencies);
	}

	rayMarchingGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/general/fullscreenTriangle.vert";
	rayMarchingGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/atmosphere/rayMarching.frag";
	rayMarchingGraphicsPipeline.renderPass = &rayMarchingRenderPass;
	rayMarchingGraphicsPipeline.viewport = &rayMarchingViewport;
	rayMarchingGraphicsPipeline.multiSample = false;
	rayMarchingGraphicsPipeline.frontFaceCCW = false;
	rayMarchingGraphicsPipeline.depthWrite = false;
	rayMarchingGraphicsPipeline.blendings.push_back({ VK_TRUE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD });
	rayMarchingGraphicsPipeline.init();

	createResources(fullscreenViewport);
}

void Atmosphere::destroy() {
	destroyResources();
	transmittanceImage.destroy();
	transmittanceGraphicsPipeline.destroy();
	dummyImage.destroy();
	transmittanceFramebuffer.destroy();
	transmittanceRenderPass.destroy();
	multiScatteringImage.destroy();
	multiScatteringComputePipeline.destroy();
	skyViewImage.destroy();
	skyViewGraphicsPipeline.destroy();
	skyViewFramebuffer.destroy();
	skyViewRenderPass.destroy();
	cameraVolumeImage.destroy();
	cameraVolumeToSampleImage.destroy();
	cameraVolumeGraphicsPipeline.destroy();
	cameraVolumeFramebuffer.destroy();
	cameraVolumeRenderPass.destroy();
	rayMarchingGraphicsPipeline.destroy();
	rayMarchingRenderPass.destroy();
}

void Atmosphere::createResources(Viewport fullscreenViewport) {
	// Ray marching
	rayMarchingViewport.init(static_cast<uint32_t>(fullscreenViewport.viewport.width), static_cast<uint32_t>(fullscreenViewport.viewport.height));

	{
		std::vector<VkImageView> framebufferAttachments;
		framebufferAttachments.push_back(sceneImage.imageView);
		rayMarchingFramebuffer.init(&rayMarchingRenderPass, framebufferAttachments, static_cast<uint32_t>(rayMarchingViewport.viewport.width), static_cast<uint32_t>(rayMarchingViewport.viewport.height), 1);
	}

	rayMarchingDescriptorSets.resize(framesInFlight);
	for (uint32_t i = 0; i < framesInFlight; i++) {
		rayMarchingDescriptorSets[i].init(&rayMarchingGraphicsPipeline, 0);

		VkDescriptorImageInfo dummyInfo = {};
		dummyInfo.sampler = trilinearEdgeBlackSampler;
		dummyInfo.imageView = dummyImage.imageView;
		dummyInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo skyViewInfo = {};
		skyViewInfo.sampler = trilinearEdgeBlackSampler;
		skyViewInfo.imageView = skyViewImage.imageView;
		skyViewInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo depthPrepassInfo = {};
		depthPrepassInfo.sampler = trilinearEdgeBlackSampler;
		depthPrepassInfo.imageView = depthPrepass.image.imageView;
		depthPrepassInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo cameraVolumeInfo = {};
		cameraVolumeInfo.sampler = trilinearEdgeBlackSampler;
		cameraVolumeInfo.imageView = cameraVolumeToSampleImage.imageView;
		cameraVolumeInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorBufferInfo cameraInfo = {};
		cameraInfo.buffer = cameraBuffers.at(i).buffer;
		cameraInfo.offset = 0;
		cameraInfo.range = sizeof(CameraUniformBufferObject);

		rayMarchingDescriptorSets[i].writesDescriptorSet.clear();
		rayMarchingDescriptorSets[i].writesDescriptorSet.shrink_to_fit();

		rayMarchingDescriptorSets[i].addWriteCombinedImageSampler(0, 1, &dummyInfo);

		rayMarchingDescriptorSets[i].addWriteCombinedImageSampler(1, 1, &dummyInfo);

		rayMarchingDescriptorSets[i].addWriteCombinedImageSampler(2, 1, &skyViewInfo);

		rayMarchingDescriptorSets[i].addWriteCombinedImageSampler(3, 1, &depthPrepassInfo);

		rayMarchingDescriptorSets[i].addWriteCombinedImageSampler(4, 1, &cameraVolumeInfo);

		rayMarchingDescriptorSets[i].addWriteUniformBuffer(5, 1, &cameraInfo);

		rayMarchingDescriptorSets[i].update();
	}
}

void Atmosphere::destroyResources() {
	rayMarchingFramebuffer.destroy();
}

void Atmosphere::draw(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex) {
	// Transmittance
	transmittanceRenderPass.begin(commandBuffer, transmittanceFramebuffer.framebuffer, { TRANSMITTANCE_WIDTH, TRANSMITTANCE_HEIGHT });
	transmittanceGraphicsPipeline.bind(commandBuffer);
	transmittanceDescriptorSet.bind(commandBuffer, &transmittanceGraphicsPipeline, 0);

	vkCmdDraw(commandBuffer->commandBuffer, 3, 1, 0, 0);

	transmittanceRenderPass.end(commandBuffer);

	// Multi scattering
	multiScatteringComputePipeline.bind(commandBuffer);
	multiScatteringDescriptorSet.bind(commandBuffer, &multiScatteringComputePipeline, 0);

	vkCmdDispatch(commandBuffer->commandBuffer, 32, 32, 1);

	computeFragmentBarrier(commandBuffer);

	// Sky view
	skyViewRenderPass.begin(commandBuffer, skyViewFramebuffer.framebuffer, { SKY_VIEW_WIDTH, SKY_VIEW_HEIGHT });
	skyViewGraphicsPipeline.bind(commandBuffer);
	skyViewDescriptorSets[frameInFlightIndex].bind(commandBuffer, &skyViewGraphicsPipeline, 0);
	skyViewGraphicsPipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 3 * sizeof(float), &mainDirectionalLightDirection);

	vkCmdDraw(commandBuffer->commandBuffer, 3, 1, 0, 0);

	skyViewRenderPass.end(commandBuffer);

	// Camera volume
	cameraVolumeRenderPass.begin(commandBuffer, cameraVolumeFramebuffer.framebuffer, { CAMERA_VOLUME_WIDTH, CAMERA_VOLUME_HEIGHT });
	cameraVolumeGraphicsPipeline.bind(commandBuffer);
	cameraVolumeDescriptorSets[frameInFlightIndex].bind(commandBuffer, &cameraVolumeGraphicsPipeline, 0);
	cameraVolumeGraphicsPipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 3 * sizeof(float), &mainDirectionalLightDirection);

	vkCmdDraw(commandBuffer->commandBuffer, 3, 32, 0, 0);

	cameraVolumeRenderPass.end(commandBuffer);

	// Ray marching
	rayMarchingRenderPass.begin(commandBuffer, rayMarchingFramebuffer.framebuffer, { static_cast<uint32_t>(rayMarchingViewport.viewport.width), static_cast<uint32_t>(rayMarchingViewport.viewport.height) });
	rayMarchingGraphicsPipeline.bind(commandBuffer);
	rayMarchingDescriptorSets[frameInFlightIndex].bind(commandBuffer, &rayMarchingGraphicsPipeline, 0);
	rayMarchingGraphicsPipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 3 * sizeof(float), &mainDirectionalLightDirection);

	vkCmdDraw(commandBuffer->commandBuffer, 3, 1, 0, 0);

	rayMarchingRenderPass.end(commandBuffer);

	transitionToGeneralLayout(commandBuffer);
}

void Atmosphere::computeFragmentBarrier(CommandBuffer* commandBuffer) {
	VkImageSubresourceRange imageSubresourceRange = {};
	imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubresourceRange.baseMipLevel = 0;
	imageSubresourceRange.levelCount = 1;
	imageSubresourceRange.baseArrayLayer = 0;
	imageSubresourceRange.layerCount = 1;

	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarrier.srcQueueFamilyIndex = physicalDevice.queueFamilyIndices.computeFamily.value();
	imageMemoryBarrier.dstQueueFamilyIndex = physicalDevice.queueFamilyIndices.graphicsFamily.value();
	imageMemoryBarrier.image = multiScatteringImage.image;
	imageMemoryBarrier.subresourceRange = imageSubresourceRange;

	vkCmdPipelineBarrier(commandBuffer->commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

void Atmosphere::transitionToGeneralLayout(CommandBuffer* commandBuffer) {
	VkImageSubresourceRange imageSubresourceRange = {};
	imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubresourceRange.baseMipLevel = 0;
	imageSubresourceRange.levelCount = 1;
	imageSubresourceRange.baseArrayLayer = 0;
	imageSubresourceRange.layerCount = 1;

	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageMemoryBarrier.srcQueueFamilyIndex = physicalDevice.queueFamilyIndices.graphicsFamily.value();
	imageMemoryBarrier.dstQueueFamilyIndex = physicalDevice.queueFamilyIndices.computeFamily.value();
	imageMemoryBarrier.image = multiScatteringImage.image;
	imageMemoryBarrier.subresourceRange = imageSubresourceRange;

	vkCmdPipelineBarrier(commandBuffer->commandBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}
