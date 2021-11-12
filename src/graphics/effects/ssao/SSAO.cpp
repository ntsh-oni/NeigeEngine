#include "SSAO.h"
#include "../../../utils/resources/BufferTools.h"
#include "../../../utils/resources/ImageTools.h"
#include "../../../graphics/resources/RendererResources.h"
#include "../../../graphics/resources/Samplers.h"
#include "../../../graphics/resources/ShaderResources.h"

void SSAO::init(int downscale, Viewport fullscreenViewport) {
	ssaoDownscale = std::max(downscale, 1);
	viewport.init(std::max(static_cast<uint32_t>(fullscreenViewport.viewport.width) / ssaoDownscale, static_cast<uint32_t>(1)), std::max(static_cast<uint32_t>(fullscreenViewport.viewport.height) / ssaoDownscale, static_cast<uint32_t>(1)));

	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, physicalDevice.colorFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		
		depthToPositionsRenderPass.init(attachments, dependencies);
	}

	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, physicalDevice.colorFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		
		depthToNormalsRenderPass.init(attachments, dependencies);
	}

	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		
		ssaoRenderPass.init(attachments, dependencies);
	}

	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });

		ssaoBlurredRenderPass.init(attachments, dependencies);
	}

	depthToPositionsGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/general/fullscreenTriangle.vert";
	depthToPositionsGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/ssao/depthToPositions.frag";
	depthToPositionsGraphicsPipeline.renderPass = &depthToPositionsRenderPass;
	depthToPositionsGraphicsPipeline.viewport = &viewport;
	depthToPositionsGraphicsPipeline.multiSample = false;
	depthToPositionsGraphicsPipeline.backfaceCulling = false;
	depthToPositionsGraphicsPipeline.depthWrite = false;
	depthToPositionsGraphicsPipeline.init();
	
	depthToNormalsGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/general/fullscreenTriangle.vert";
	depthToNormalsGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/ssao/depthToNormals.frag";
	depthToNormalsGraphicsPipeline.renderPass = &depthToNormalsRenderPass;
	depthToNormalsGraphicsPipeline.viewport = &viewport;
	depthToNormalsGraphicsPipeline.multiSample = false;
	depthToNormalsGraphicsPipeline.backfaceCulling = false;
	depthToNormalsGraphicsPipeline.depthWrite = false;
	depthToNormalsGraphicsPipeline.init();

	ssaoGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/general/fullscreenTriangle.vert";
	ssaoGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/ssao/ssao.frag";
	ssaoGraphicsPipeline.renderPass = &ssaoRenderPass;
	ssaoGraphicsPipeline.viewport = &viewport;
	ssaoGraphicsPipeline.multiSample = false;
	ssaoGraphicsPipeline.backfaceCulling = false;
	ssaoGraphicsPipeline.depthWrite = false;
	ssaoGraphicsPipeline.init();

	ssaoBlurredGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/general/fullscreenTriangle.vert";
	ssaoBlurredGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/ssao/ssaoBlur.frag";
	ssaoBlurredGraphicsPipeline.renderPass = &ssaoBlurredRenderPass;
	ssaoBlurredGraphicsPipeline.viewport = &viewport;
	ssaoBlurredGraphicsPipeline.multiSample = false;
	ssaoBlurredGraphicsPipeline.backfaceCulling = false;
	ssaoBlurredGraphicsPipeline.depthWrite = false;
	ssaoBlurredGraphicsPipeline.init();

	BufferTools::createUniformBuffer(sampleKernel.buffer, SSAOSAMPLES * 4 * sizeof(float), &sampleKernel.memoryInfo);

	createRandomTexture();

	createResources(fullscreenViewport);
}

void SSAO::destroy() {
	destroyResources();
	sampleKernel.destroy();
	randomTexture.destroy();
	depthToPositionsRenderPass.destroy();
	depthToNormalsRenderPass.destroy();
	ssaoRenderPass.destroy();
	ssaoBlurredRenderPass.destroy();
	depthToNormalsGraphicsPipeline.destroy();
	depthToPositionsGraphicsPipeline.destroy();
	ssaoGraphicsPipeline.destroy();
	ssaoBlurredGraphicsPipeline.destroy();
}

void SSAO::createResources(Viewport fullscreenViewport) {
	viewport.init(std::max(static_cast<uint32_t>(fullscreenViewport.viewport.width) / ssaoDownscale, static_cast<uint32_t>(1)), std::max(static_cast<uint32_t>(fullscreenViewport.viewport.height) / ssaoDownscale, static_cast<uint32_t>(1)));
	
	// Images
	ImageTools::createImage(&depthToPositionsImage.image, 1, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1, VK_SAMPLE_COUNT_1_BIT, physicalDevice.colorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthToPositionsImage.memoryInfo);
	ImageTools::createImageView(&depthToPositionsImage.imageView, depthToPositionsImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);

	ImageTools::createImage(&depthToNormalsImage.image, 1, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1, VK_SAMPLE_COUNT_1_BIT, physicalDevice.colorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthToNormalsImage.memoryInfo);
	ImageTools::createImageView(&depthToNormalsImage.imageView, depthToNormalsImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);

	ImageTools::createImage(&ssaoImage.image, 1, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &ssaoImage.memoryInfo);
	ImageTools::createImageView(&ssaoImage.imageView, ssaoImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

	ImageTools::createImage(&ssaoBlurredImage.image, 1, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &ssaoBlurredImage.memoryInfo);
	ImageTools::createImageView(&ssaoBlurredImage.imageView, ssaoBlurredImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

	// Framebuffers
	{
		std::vector<VkImageView> framebufferAttachements;
		framebufferAttachements.push_back(depthToPositionsImage.imageView);
		depthToPositionsFramebuffer.init(&depthToPositionsRenderPass, framebufferAttachements, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1);
	}

	{
		std::vector<VkImageView> framebufferAttachements;
		framebufferAttachements.push_back(depthToNormalsImage.imageView);
		depthToNormalsFramebuffer.init(&depthToNormalsRenderPass, framebufferAttachements, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1);
	}

	{
		std::vector<VkImageView> framebufferAttachements;
		framebufferAttachements.push_back(ssaoImage.imageView);
		ssaoFramebuffer.init(&ssaoRenderPass, framebufferAttachements, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1);
	}

	{
		std::vector<VkImageView> framebufferAttachements;
		framebufferAttachements.push_back(ssaoBlurredImage.imageView);
		ssaoBlurredFramebuffer.init(&ssaoBlurredRenderPass, framebufferAttachements, static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1);
	}

	// Descriptor sets
	{
		depthToPositionsDescriptorSets.resize(framesInFlight);

		for (size_t i = 0; i < framesInFlight; i++) {
			depthToPositionsDescriptorSets[i].init(&depthToPositionsGraphicsPipeline, 0);

			VkDescriptorImageInfo depthPrepassInfo = {};
			depthPrepassInfo.sampler = nearestOffscreenSampler;
			depthPrepassInfo.imageView = depthPrepass.image.imageView;
			depthPrepassInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

			VkDescriptorBufferInfo cameraInfo = {};
			cameraInfo.buffer = cameraBuffers[i].buffer;
			cameraInfo.offset = 0;
			cameraInfo.range = sizeof(CameraUniformBufferObject);

			std::vector<VkWriteDescriptorSet> writesDescriptorSet;

			VkWriteDescriptorSet depthPrepassWriteDescriptorSet = {};
			depthPrepassWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			depthPrepassWriteDescriptorSet.pNext = nullptr;
			depthPrepassWriteDescriptorSet.dstSet = depthToPositionsDescriptorSets[i].descriptorSet;
			depthPrepassWriteDescriptorSet.dstBinding = 0;
			depthPrepassWriteDescriptorSet.dstArrayElement = 0;
			depthPrepassWriteDescriptorSet.descriptorCount = 1;
			depthPrepassWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			depthPrepassWriteDescriptorSet.pImageInfo = &depthPrepassInfo;
			depthPrepassWriteDescriptorSet.pBufferInfo = nullptr;
			depthPrepassWriteDescriptorSet.pTexelBufferView = nullptr;
			writesDescriptorSet.push_back(depthPrepassWriteDescriptorSet);

			VkWriteDescriptorSet cameraWriteDescriptorSet = {};
			cameraWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			cameraWriteDescriptorSet.pNext = nullptr;
			cameraWriteDescriptorSet.dstSet = depthToPositionsDescriptorSets[i].descriptorSet;
			cameraWriteDescriptorSet.dstBinding = 1;
			cameraWriteDescriptorSet.dstArrayElement = 0;
			cameraWriteDescriptorSet.descriptorCount = 1;
			cameraWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			cameraWriteDescriptorSet.pImageInfo = nullptr;
			cameraWriteDescriptorSet.pBufferInfo = &cameraInfo;
			cameraWriteDescriptorSet.pTexelBufferView = nullptr;
			writesDescriptorSet.push_back(cameraWriteDescriptorSet);

			depthToPositionsDescriptorSets[i].update(writesDescriptorSet);
		}
	}

	{
		depthToNormalsDescriptorSets.resize(framesInFlight);

		for (size_t i = 0; i < framesInFlight; i++) {
			depthToNormalsDescriptorSets[i].init(&depthToNormalsGraphicsPipeline, 0);

			VkDescriptorImageInfo depthPrepassInfo = {};
			depthPrepassInfo.sampler = nearestOffscreenSampler;
			depthPrepassInfo.imageView = depthPrepass.image.imageView;
			depthPrepassInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

			VkDescriptorBufferInfo cameraInfo = {};
			cameraInfo.buffer = cameraBuffers[i].buffer;
			cameraInfo.offset = 0;
			cameraInfo.range = sizeof(CameraUniformBufferObject);

			std::vector<VkWriteDescriptorSet> writesDescriptorSet;

			VkWriteDescriptorSet depthPrepassWriteDescriptorSet = {};
			depthPrepassWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			depthPrepassWriteDescriptorSet.pNext = nullptr;
			depthPrepassWriteDescriptorSet.dstSet = depthToNormalsDescriptorSets[i].descriptorSet;
			depthPrepassWriteDescriptorSet.dstBinding = 0;
			depthPrepassWriteDescriptorSet.dstArrayElement = 0;
			depthPrepassWriteDescriptorSet.descriptorCount = 1;
			depthPrepassWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			depthPrepassWriteDescriptorSet.pImageInfo = &depthPrepassInfo;
			depthPrepassWriteDescriptorSet.pBufferInfo = nullptr;
			depthPrepassWriteDescriptorSet.pTexelBufferView = nullptr;
			writesDescriptorSet.push_back(depthPrepassWriteDescriptorSet);

			VkWriteDescriptorSet cameraWriteDescriptorSet = {};
			cameraWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			cameraWriteDescriptorSet.pNext = nullptr;
			cameraWriteDescriptorSet.dstSet = depthToNormalsDescriptorSets[i].descriptorSet;
			cameraWriteDescriptorSet.dstBinding = 1;
			cameraWriteDescriptorSet.dstArrayElement = 0;
			cameraWriteDescriptorSet.descriptorCount = 1;
			cameraWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			cameraWriteDescriptorSet.pImageInfo = nullptr;
			cameraWriteDescriptorSet.pBufferInfo = &cameraInfo;
			cameraWriteDescriptorSet.pTexelBufferView = nullptr;
			writesDescriptorSet.push_back(cameraWriteDescriptorSet);

			depthToNormalsDescriptorSets[i].update(writesDescriptorSet);
		}
	}

	{
		ssaoDescriptorSets.resize(framesInFlight);

		for (size_t i = 0; i < framesInFlight; i++) {
			ssaoDescriptorSets[i].init(&ssaoGraphicsPipeline, 0);

			VkDescriptorImageInfo positionInfo = {};
			positionInfo.sampler = nearestOffscreenSampler;
			positionInfo.imageView = depthToPositionsImage.imageView;
			positionInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkDescriptorImageInfo normalInfo = {};
			normalInfo.sampler = nearestOffscreenSampler;
			normalInfo.imageView = depthToNormalsImage.imageView;
			normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkDescriptorImageInfo randomTextureInfo = {};
			randomTextureInfo.sampler = nearestRepeatOffscreenSampler;
			randomTextureInfo.imageView = randomTexture.imageView;
			randomTextureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkDescriptorBufferInfo sampleKernelInfo = {};
			sampleKernelInfo.buffer = sampleKernel.buffer;
			sampleKernelInfo.offset = 0;
			sampleKernelInfo.range = SSAOSAMPLES * 4 * sizeof(float);

			VkDescriptorBufferInfo cameraInfo = {};
			cameraInfo.buffer = cameraBuffers[i].buffer;
			cameraInfo.offset = 0;
			cameraInfo.range = sizeof(CameraUniformBufferObject);

			std::vector<VkWriteDescriptorSet> writesDescriptorSet;

			VkWriteDescriptorSet positionWriteDescriptorSet = {};
			positionWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			positionWriteDescriptorSet.pNext = nullptr;
			positionWriteDescriptorSet.dstSet = ssaoDescriptorSets[i].descriptorSet;
			positionWriteDescriptorSet.dstBinding = 0;
			positionWriteDescriptorSet.dstArrayElement = 0;
			positionWriteDescriptorSet.descriptorCount = 1;
			positionWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			positionWriteDescriptorSet.pImageInfo = &positionInfo;
			positionWriteDescriptorSet.pBufferInfo = nullptr;
			positionWriteDescriptorSet.pTexelBufferView = nullptr;
			writesDescriptorSet.push_back(positionWriteDescriptorSet);

			VkWriteDescriptorSet normalWriteDescriptorSet = {};
			normalWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			normalWriteDescriptorSet.pNext = nullptr;
			normalWriteDescriptorSet.dstSet = ssaoDescriptorSets[i].descriptorSet;
			normalWriteDescriptorSet.dstBinding = 1;
			normalWriteDescriptorSet.dstArrayElement = 0;
			normalWriteDescriptorSet.descriptorCount = 1;
			normalWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			normalWriteDescriptorSet.pImageInfo = &normalInfo;
			normalWriteDescriptorSet.pBufferInfo = nullptr;
			normalWriteDescriptorSet.pTexelBufferView = nullptr;
			writesDescriptorSet.push_back(normalWriteDescriptorSet);

			VkWriteDescriptorSet randomTextureWriteDescriptorSet = {};
			randomTextureWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			randomTextureWriteDescriptorSet.pNext = nullptr;
			randomTextureWriteDescriptorSet.dstSet = ssaoDescriptorSets[i].descriptorSet;
			randomTextureWriteDescriptorSet.dstBinding = 2;
			randomTextureWriteDescriptorSet.dstArrayElement = 0;
			randomTextureWriteDescriptorSet.descriptorCount = 1;
			randomTextureWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			randomTextureWriteDescriptorSet.pImageInfo = &randomTextureInfo;
			randomTextureWriteDescriptorSet.pBufferInfo = nullptr;
			randomTextureWriteDescriptorSet.pTexelBufferView = nullptr;
			writesDescriptorSet.push_back(randomTextureWriteDescriptorSet);

			VkWriteDescriptorSet sampleKernelWriteDescriptorSet = {};
			sampleKernelWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			sampleKernelWriteDescriptorSet.pNext = nullptr;
			sampleKernelWriteDescriptorSet.dstSet = ssaoDescriptorSets[i].descriptorSet;
			sampleKernelWriteDescriptorSet.dstBinding = 3;
			sampleKernelWriteDescriptorSet.dstArrayElement = 0;
			sampleKernelWriteDescriptorSet.descriptorCount = 1;
			sampleKernelWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			sampleKernelWriteDescriptorSet.pImageInfo = nullptr;
			sampleKernelWriteDescriptorSet.pBufferInfo = &sampleKernelInfo;
			sampleKernelWriteDescriptorSet.pTexelBufferView = nullptr;
			writesDescriptorSet.push_back(sampleKernelWriteDescriptorSet);

			VkWriteDescriptorSet cameraWriteDescriptorSet = {};
			cameraWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			cameraWriteDescriptorSet.pNext = nullptr;
			cameraWriteDescriptorSet.dstSet = ssaoDescriptorSets[i].descriptorSet;
			cameraWriteDescriptorSet.dstBinding = 4;
			cameraWriteDescriptorSet.dstArrayElement = 0;
			cameraWriteDescriptorSet.descriptorCount = 1;
			cameraWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			cameraWriteDescriptorSet.pImageInfo = nullptr;
			cameraWriteDescriptorSet.pBufferInfo = &cameraInfo;
			cameraWriteDescriptorSet.pTexelBufferView = nullptr;
			writesDescriptorSet.push_back(cameraWriteDescriptorSet);

			ssaoDescriptorSets[i].update(writesDescriptorSet);
		}
	}

	{
		ssaoBlurredDescriptorSet.init(&ssaoBlurredGraphicsPipeline, 0);

		VkDescriptorImageInfo ssaoInfo = {};
		ssaoInfo.sampler = nearestOffscreenSampler;
		ssaoInfo.imageView = ssaoImage.imageView;
		ssaoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet ssaoWriteDescriptorSet = {};
		ssaoWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		ssaoWriteDescriptorSet.pNext = nullptr;
		ssaoWriteDescriptorSet.dstSet = ssaoBlurredDescriptorSet.descriptorSet;
		ssaoWriteDescriptorSet.dstBinding = 0;
		ssaoWriteDescriptorSet.dstArrayElement = 0;
		ssaoWriteDescriptorSet.descriptorCount = 1;
		ssaoWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		ssaoWriteDescriptorSet.pImageInfo = &ssaoInfo;
		ssaoWriteDescriptorSet.pBufferInfo = nullptr;
		ssaoWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(ssaoWriteDescriptorSet);

		ssaoBlurredDescriptorSet.update(writesDescriptorSet);
	}
}

void SSAO::destroyResources() {
	depthToPositionsImage.destroy();
	depthToNormalsImage.destroy();
	ssaoImage.destroy();
	ssaoBlurredImage.destroy();
	depthToPositionsFramebuffer.destroy();
	depthToNormalsFramebuffer.destroy();
	ssaoFramebuffer.destroy();
	ssaoBlurredFramebuffer.destroy();
}

void SSAO::createRandomTexture() {
	std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
	std::default_random_engine gen;

	std::vector<glm::vec4> samples;
	float randomRotations[64];

	for (unsigned char i = 0; i < SSAOSAMPLES; i++) {
		// Sample kernel
		glm::vec3 sample{ randomFloats(gen) * 2.0f - 1.0f,
		randomFloats(gen) * 2.0f - 1.0f,
		randomFloats(gen) };
		sample = glm::normalize(sample);
		sample *= randomFloats(gen);

		float scale = static_cast<float>(i) / 64.0f;
		// Lerp
		scale = 0.1f + (scale * scale) * (1.0f - 0.1f);

		sample *= scale;

		samples.push_back(glm::vec4(sample, 0.0f));
	}

	VkDeviceSize size = SSAOSAMPLES * 4 * sizeof(float);
	memcpy(reinterpret_cast<void*>(reinterpret_cast<char*>(sampleKernel.memoryInfo.data) + sampleKernel.memoryInfo.offset), samples.data(), size);

	for (unsigned char i = 0; i < 16; i++) {
		// Random rotation
		randomRotations[(i * 4) + 0] = randomFloats(gen) * 2.0f - 1.0f;
		randomRotations[(i * 4) + 1] = randomFloats(gen) * 2.0f - 1.0f;
		randomRotations[(i * 4) + 2] = 0.0f;
		randomRotations[(i * 4) + 3] = 1.0f;
	}

	ImageTools::loadColorArray(randomRotations, &randomTexture.image, 4, 4, physicalDevice.colorFormat, &randomTexture.mipmapLevels, &randomTexture.memoryInfo);
	ImageTools::createImageView(&randomTexture.imageView, randomTexture.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

void SSAO::draw(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex) {
	// Depth to positions
	depthToPositionsRenderPass.begin(commandBuffer, depthToPositionsFramebuffer.framebuffer, { static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height) });

	depthToPositionsGraphicsPipeline.bind(commandBuffer);
	depthToPositionsDescriptorSets[frameInFlightIndex].bind(commandBuffer, &depthToPositionsGraphicsPipeline, 0);

	vkCmdDraw(commandBuffer->commandBuffer, 3, 1, 0, 0);

	depthToPositionsRenderPass.end(commandBuffer);

	// Depth to normals
	depthToNormalsRenderPass.begin(commandBuffer, depthToNormalsFramebuffer.framebuffer, { static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height) });

	depthToNormalsGraphicsPipeline.bind(commandBuffer);
	depthToNormalsDescriptorSets[frameInFlightIndex].bind(commandBuffer, &depthToNormalsGraphicsPipeline, 0);

	vkCmdDraw(commandBuffer->commandBuffer, 3, 1, 0, 0);

	depthToNormalsRenderPass.end(commandBuffer);

	// SSAO
	ssaoRenderPass.begin(commandBuffer, ssaoFramebuffer.framebuffer, { static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height) });

	ssaoGraphicsPipeline.bind(commandBuffer);
	ssaoDescriptorSets[frameInFlightIndex].bind(commandBuffer, &ssaoGraphicsPipeline, 0);

	vkCmdDraw(commandBuffer->commandBuffer, 3, 1, 0, 0);

	ssaoRenderPass.end(commandBuffer);

	// SSAO blurred
	ssaoBlurredRenderPass.begin(commandBuffer, ssaoBlurredFramebuffer.framebuffer, { static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height) });

	ssaoBlurredGraphicsPipeline.bind(commandBuffer);
	ssaoBlurredDescriptorSet.bind(commandBuffer, &ssaoBlurredGraphicsPipeline, 0);

	vkCmdDraw(commandBuffer->commandBuffer, 3, 1, 0, 0);

	ssaoBlurredRenderPass.end(commandBuffer);
}