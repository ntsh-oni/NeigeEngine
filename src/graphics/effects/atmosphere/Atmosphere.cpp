#include "Atmosphere.h"
#include "../../../utils/resources/ImageTools.h"
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
	transmittanceGraphicsPipeline.backfaceCulling = false;
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

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet dummyTransmittanceWriteDescriptorSet = {};
		dummyTransmittanceWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		dummyTransmittanceWriteDescriptorSet.pNext = nullptr;
		dummyTransmittanceWriteDescriptorSet.dstSet = transmittanceDescriptorSet.descriptorSet;
		dummyTransmittanceWriteDescriptorSet.dstBinding = 0;
		dummyTransmittanceWriteDescriptorSet.dstArrayElement = 0;
		dummyTransmittanceWriteDescriptorSet.descriptorCount = 1;
		dummyTransmittanceWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		dummyTransmittanceWriteDescriptorSet.pImageInfo = &dummyInfo;
		dummyTransmittanceWriteDescriptorSet.pBufferInfo = nullptr;
		dummyTransmittanceWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(dummyTransmittanceWriteDescriptorSet);

		VkWriteDescriptorSet dummyMultiScatteringWriteDescriptorSet = {};
		dummyMultiScatteringWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		dummyMultiScatteringWriteDescriptorSet.pNext = nullptr;
		dummyMultiScatteringWriteDescriptorSet.dstSet = transmittanceDescriptorSet.descriptorSet;
		dummyMultiScatteringWriteDescriptorSet.dstBinding = 1;
		dummyMultiScatteringWriteDescriptorSet.dstArrayElement = 0;
		dummyMultiScatteringWriteDescriptorSet.descriptorCount = 1;
		dummyMultiScatteringWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		dummyMultiScatteringWriteDescriptorSet.pImageInfo = &dummyInfo;
		dummyMultiScatteringWriteDescriptorSet.pBufferInfo = nullptr;
		dummyMultiScatteringWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(dummyMultiScatteringWriteDescriptorSet);

		transmittanceDescriptorSet.update(writesDescriptorSet);
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

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet transmittanceWriteDescriptorSet = {};
		transmittanceWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		transmittanceWriteDescriptorSet.pNext = nullptr;
		transmittanceWriteDescriptorSet.dstSet = multiScatteringDescriptorSet.descriptorSet;
		transmittanceWriteDescriptorSet.dstBinding = 0;
		transmittanceWriteDescriptorSet.dstArrayElement = 0;
		transmittanceWriteDescriptorSet.descriptorCount = 1;
		transmittanceWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		transmittanceWriteDescriptorSet.pImageInfo = &transmittanceInfo;
		transmittanceWriteDescriptorSet.pBufferInfo = nullptr;
		transmittanceWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(transmittanceWriteDescriptorSet);

		VkWriteDescriptorSet dummyMultiScatteringWriteDescriptorSet = {};
		dummyMultiScatteringWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		dummyMultiScatteringWriteDescriptorSet.pNext = nullptr;
		dummyMultiScatteringWriteDescriptorSet.dstSet = multiScatteringDescriptorSet.descriptorSet;
		dummyMultiScatteringWriteDescriptorSet.dstBinding = 1;
		dummyMultiScatteringWriteDescriptorSet.dstArrayElement = 0;
		dummyMultiScatteringWriteDescriptorSet.descriptorCount = 1;
		dummyMultiScatteringWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		dummyMultiScatteringWriteDescriptorSet.pImageInfo = &dummyInfo;
		dummyMultiScatteringWriteDescriptorSet.pBufferInfo = nullptr;
		dummyMultiScatteringWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(dummyMultiScatteringWriteDescriptorSet);

		VkWriteDescriptorSet multiScatteringWriteDescriptorSet = {};
		multiScatteringWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		multiScatteringWriteDescriptorSet.pNext = nullptr;
		multiScatteringWriteDescriptorSet.dstSet = multiScatteringDescriptorSet.descriptorSet;
		multiScatteringWriteDescriptorSet.dstBinding = 2;
		multiScatteringWriteDescriptorSet.dstArrayElement = 0;
		multiScatteringWriteDescriptorSet.descriptorCount = 1;
		multiScatteringWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		multiScatteringWriteDescriptorSet.pImageInfo = &multiScatteringInfo;
		multiScatteringWriteDescriptorSet.pBufferInfo = nullptr;
		multiScatteringWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(multiScatteringWriteDescriptorSet);

		multiScatteringDescriptorSet.update(writesDescriptorSet);
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
	skyViewGraphicsPipeline.backfaceCulling = false;
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

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet transmittanceWriteDescriptorSet = {};
		transmittanceWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		transmittanceWriteDescriptorSet.pNext = nullptr;
		transmittanceWriteDescriptorSet.dstSet = skyViewDescriptorSets[i].descriptorSet;
		transmittanceWriteDescriptorSet.dstBinding = 0;
		transmittanceWriteDescriptorSet.dstArrayElement = 0;
		transmittanceWriteDescriptorSet.descriptorCount = 1;
		transmittanceWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		transmittanceWriteDescriptorSet.pImageInfo = &transmittanceInfo;
		transmittanceWriteDescriptorSet.pBufferInfo = nullptr;
		transmittanceWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(transmittanceWriteDescriptorSet);

		VkWriteDescriptorSet multiScatteringWriteDescriptorSet = {};
		multiScatteringWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		multiScatteringWriteDescriptorSet.pNext = nullptr;
		multiScatteringWriteDescriptorSet.dstSet = skyViewDescriptorSets[i].descriptorSet;
		multiScatteringWriteDescriptorSet.dstBinding = 1;
		multiScatteringWriteDescriptorSet.dstArrayElement = 0;
		multiScatteringWriteDescriptorSet.descriptorCount = 1;
		multiScatteringWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		multiScatteringWriteDescriptorSet.pImageInfo = &multiScatteringInfo;
		multiScatteringWriteDescriptorSet.pBufferInfo = nullptr;
		multiScatteringWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(multiScatteringWriteDescriptorSet);

		VkWriteDescriptorSet cameraWriteDescriptorSet = {};
		cameraWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraWriteDescriptorSet.pNext = nullptr;
		cameraWriteDescriptorSet.dstSet = skyViewDescriptorSets[i].descriptorSet;
		cameraWriteDescriptorSet.dstBinding = 2;
		cameraWriteDescriptorSet.dstArrayElement = 0;
		cameraWriteDescriptorSet.descriptorCount = 1;
		cameraWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraWriteDescriptorSet.pImageInfo = nullptr;
		cameraWriteDescriptorSet.pBufferInfo = &cameraInfo;
		cameraWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(cameraWriteDescriptorSet);

		skyViewDescriptorSets[i].update(writesDescriptorSet);
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
	cameraVolumeGraphicsPipeline.backfaceCulling = false;
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

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet transmittanceWriteDescriptorSet = {};
		transmittanceWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		transmittanceWriteDescriptorSet.pNext = nullptr;
		transmittanceWriteDescriptorSet.dstSet = cameraVolumeDescriptorSets[i].descriptorSet;
		transmittanceWriteDescriptorSet.dstBinding = 0;
		transmittanceWriteDescriptorSet.dstArrayElement = 0;
		transmittanceWriteDescriptorSet.descriptorCount = 1;
		transmittanceWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		transmittanceWriteDescriptorSet.pImageInfo = &transmittanceInfo;
		transmittanceWriteDescriptorSet.pBufferInfo = nullptr;
		transmittanceWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(transmittanceWriteDescriptorSet);

		VkWriteDescriptorSet multiScatteringWriteDescriptorSet = {};
		multiScatteringWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		multiScatteringWriteDescriptorSet.pNext = nullptr;
		multiScatteringWriteDescriptorSet.dstSet = cameraVolumeDescriptorSets[i].descriptorSet;
		multiScatteringWriteDescriptorSet.dstBinding = 1;
		multiScatteringWriteDescriptorSet.dstArrayElement = 0;
		multiScatteringWriteDescriptorSet.descriptorCount = 1;
		multiScatteringWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		multiScatteringWriteDescriptorSet.pImageInfo = &multiScatteringInfo;
		multiScatteringWriteDescriptorSet.pBufferInfo = nullptr;
		multiScatteringWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(multiScatteringWriteDescriptorSet);

		VkWriteDescriptorSet cameraWriteDescriptorSet = {};
		cameraWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraWriteDescriptorSet.pNext = nullptr;
		cameraWriteDescriptorSet.dstSet = cameraVolumeDescriptorSets[i].descriptorSet;
		cameraWriteDescriptorSet.dstBinding = 2;
		cameraWriteDescriptorSet.dstArrayElement = 0;
		cameraWriteDescriptorSet.descriptorCount = 1;
		cameraWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraWriteDescriptorSet.pImageInfo = nullptr;
		cameraWriteDescriptorSet.pBufferInfo = &cameraInfo;
		cameraWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(cameraWriteDescriptorSet);

		cameraVolumeDescriptorSets[i].update(writesDescriptorSet);
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
	rayMarchingGraphicsPipeline.backfaceCulling = false;
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

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet dummyWriteDescriptorSet = {};
		dummyWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		dummyWriteDescriptorSet.pNext = nullptr;
		dummyWriteDescriptorSet.dstSet = rayMarchingDescriptorSets[i].descriptorSet;
		dummyWriteDescriptorSet.dstBinding = 0;
		dummyWriteDescriptorSet.dstArrayElement = 0;
		dummyWriteDescriptorSet.descriptorCount = 1;
		dummyWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		dummyWriteDescriptorSet.pImageInfo = &dummyInfo;
		dummyWriteDescriptorSet.pBufferInfo = nullptr;
		dummyWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(dummyWriteDescriptorSet);

		VkWriteDescriptorSet multiScatteringWriteDescriptorSet = {};
		multiScatteringWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		multiScatteringWriteDescriptorSet.pNext = nullptr;
		multiScatteringWriteDescriptorSet.dstSet = rayMarchingDescriptorSets[i].descriptorSet;
		multiScatteringWriteDescriptorSet.dstBinding = 1;
		multiScatteringWriteDescriptorSet.dstArrayElement = 0;
		multiScatteringWriteDescriptorSet.descriptorCount = 1;
		multiScatteringWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		multiScatteringWriteDescriptorSet.pImageInfo = &dummyInfo;
		multiScatteringWriteDescriptorSet.pBufferInfo = nullptr;
		multiScatteringWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(multiScatteringWriteDescriptorSet);

		VkWriteDescriptorSet skyViewWriteDescriptorSet = {};
		skyViewWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		skyViewWriteDescriptorSet.pNext = nullptr;
		skyViewWriteDescriptorSet.dstSet = rayMarchingDescriptorSets[i].descriptorSet;
		skyViewWriteDescriptorSet.dstBinding = 2;
		skyViewWriteDescriptorSet.dstArrayElement = 0;
		skyViewWriteDescriptorSet.descriptorCount = 1;
		skyViewWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		skyViewWriteDescriptorSet.pImageInfo = &skyViewInfo;
		skyViewWriteDescriptorSet.pBufferInfo = nullptr;
		skyViewWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(skyViewWriteDescriptorSet);

		VkWriteDescriptorSet depthPrepassWriteDescriptorSet = {};
		depthPrepassWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		depthPrepassWriteDescriptorSet.pNext = nullptr;
		depthPrepassWriteDescriptorSet.dstSet = rayMarchingDescriptorSets[i].descriptorSet;
		depthPrepassWriteDescriptorSet.dstBinding = 3;
		depthPrepassWriteDescriptorSet.dstArrayElement = 0;
		depthPrepassWriteDescriptorSet.descriptorCount = 1;
		depthPrepassWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		depthPrepassWriteDescriptorSet.pImageInfo = &depthPrepassInfo;
		depthPrepassWriteDescriptorSet.pBufferInfo = nullptr;
		depthPrepassWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(depthPrepassWriteDescriptorSet);

		VkWriteDescriptorSet cameraVolumeWriteDescriptorSet = {};
		cameraVolumeWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraVolumeWriteDescriptorSet.pNext = nullptr;
		cameraVolumeWriteDescriptorSet.dstSet = rayMarchingDescriptorSets[i].descriptorSet;
		cameraVolumeWriteDescriptorSet.dstBinding = 4;
		cameraVolumeWriteDescriptorSet.dstArrayElement = 0;
		cameraVolumeWriteDescriptorSet.descriptorCount = 1;
		cameraVolumeWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		cameraVolumeWriteDescriptorSet.pImageInfo = &cameraVolumeInfo;
		cameraVolumeWriteDescriptorSet.pBufferInfo = nullptr;
		cameraVolumeWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(cameraVolumeWriteDescriptorSet);

		VkWriteDescriptorSet cameraWriteDescriptorSet = {};
		cameraWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraWriteDescriptorSet.pNext = nullptr;
		cameraWriteDescriptorSet.dstSet = rayMarchingDescriptorSets[i].descriptorSet;
		cameraWriteDescriptorSet.dstBinding = 5;
		cameraWriteDescriptorSet.dstArrayElement = 0;
		cameraWriteDescriptorSet.descriptorCount = 1;
		cameraWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraWriteDescriptorSet.pImageInfo = nullptr;
		cameraWriteDescriptorSet.pBufferInfo = &cameraInfo;
		cameraWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(cameraWriteDescriptorSet);

		rayMarchingDescriptorSets[i].update(writesDescriptorSet);
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
