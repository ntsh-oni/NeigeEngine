#include "Envmap.h"
#include "vulkan/vulkan.h"
#include "../../../utils/resources/BufferTools.h"
#include "../../../utils/resources/ImageTools.h"
#include "../../../graphics/resources/RendererResources.h"
#include "../../../graphics/resources/Samplers.h"
#include "../../../graphics/resources/ShaderResources.h"

void Envmap::init(std::string filePath, Viewport* fullscreenViewport, RenderPass* opaqueSceneRenderPass) {
	if (FileTools::exists(filePath)) {
		ImageTools::loadHDREnvmap(filePath, &envmapImage.image, physicalDevice.colorFormat, &envmapImage.memoryInfo);
	}
	else {
		if (filePath != "") {
			NEIGE_WARNING("Envmap " + filePath + " does not exist.");
		}
		float defaultEnvmap[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		ImageTools::loadColorForEnvmap(defaultEnvmap, &envmapImage.image, physicalDevice.colorFormat, &envmapImage.mipmapLevels, &envmapImage.memoryInfo);
	}
	ImageTools::createImageView(&envmapImage.imageView, envmapImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);

	uint32_t skyboxMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(ENVMAP_WIDTH, ENVMAP_HEIGHT)))) + 1;
	ImageTools::createImage(&skyboxImage.image, 6, ENVMAP_WIDTH, ENVMAP_HEIGHT, skyboxMipLevels, VK_SAMPLE_COUNT_1_BIT, physicalDevice.colorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &skyboxImage.memoryInfo);
	ImageTools::createImageView(&skyboxImage.imageView, skyboxImage.image, 0, 6, 0, skyboxMipLevels, VK_IMAGE_VIEW_TYPE_CUBE, physicalDevice.colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	ImageTools::transitionLayout(skyboxImage.image, physicalDevice.colorFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, skyboxMipLevels, 6);

	float defaultSkyboxColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	ImageTools::loadColorForEnvmap(defaultSkyboxColor, &defaultSkybox.image, physicalDevice.colorFormat, &defaultSkybox.mipmapLevels, &defaultSkybox.memoryInfo);
	ImageTools::createImageView(&defaultSkybox.imageView, defaultSkybox.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);

	std::vector<uint32_t> cubeIndices;
	cubeIndices.resize(cubeVertices.size());
	std::iota(cubeIndices.begin(), cubeIndices.end(), 0);

	Buffer stagingVertexBuffer;
	VkDeviceSize size = cubeVertices.size() * sizeof(Vertex);
	BufferTools::createStagingBuffer(stagingVertexBuffer.buffer, size, &stagingVertexBuffer.memoryInfo);
	memcpy(reinterpret_cast<void*>(reinterpret_cast<char*>(stagingVertexBuffer.memoryInfo.data) + stagingVertexBuffer.memoryInfo.offset), cubeVertices.data(), static_cast<size_t>(size));
	BufferTools::createBuffer(cubeVertexBuffer.buffer, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &cubeVertexBuffer.memoryInfo);
	BufferTools::copyBuffer(stagingVertexBuffer.buffer, cubeVertexBuffer.buffer, size);
	stagingVertexBuffer.destroy();

	Buffer stagingIndexBuffer;
	size = cubeIndices.size() * sizeof(uint32_t);
	BufferTools::createStagingBuffer(stagingIndexBuffer.buffer, size, &stagingIndexBuffer.memoryInfo);
	memcpy(reinterpret_cast<void*>(reinterpret_cast<char*>(stagingIndexBuffer.memoryInfo.data) + stagingIndexBuffer.memoryInfo.offset), cubeIndices.data(), static_cast<size_t>(size));
	BufferTools::createBuffer(cubeIndexBuffer.buffer, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &cubeIndexBuffer.memoryInfo);
	BufferTools::copyBuffer(stagingIndexBuffer.buffer, cubeIndexBuffer.buffer, size);
	stagingIndexBuffer.destroy();

	equiRectangleToCubemap();
	createDiffuseIradiance();
	createPrefilter();
	createBRDFConvolution();

	// Skybox
	skyboxGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/envmap/skybox.vert";
	skyboxGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/envmap/skybox.frag";
	skyboxGraphicsPipeline.renderPass = opaqueSceneRenderPass;
	skyboxGraphicsPipeline.multiSample = false;
	skyboxGraphicsPipeline.viewport = fullscreenViewport;
	skyboxGraphicsPipeline.depthCompare = Compare::LESS_OR_EQUAL;
	skyboxGraphicsPipeline.depthWrite = false;
	skyboxGraphicsPipeline.init();

	skyboxDescriptorSets.resize(framesInFlight);
	for (uint32_t i = 0; i < framesInFlight; i++) {
		skyboxDescriptorSets[i].init(&skyboxGraphicsPipeline, 0);

		VkDescriptorBufferInfo cameraInfo = {};
		cameraInfo.buffer = cameraBuffers.at(i).buffer;
		cameraInfo.offset = 0;
		cameraInfo.range = sizeof(CameraUniformBufferObject);

		VkDescriptorImageInfo skyboxInfo = {};
		skyboxInfo.sampler = trilinearOffscreenSampler;
		skyboxInfo.imageView = envmap.skyboxImage.imageView;
		skyboxInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet cameraWriteDescriptorSet = {};
		cameraWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraWriteDescriptorSet.pNext = nullptr;
		cameraWriteDescriptorSet.dstSet = skyboxDescriptorSets[i].descriptorSet;
		cameraWriteDescriptorSet.dstBinding = 0;
		cameraWriteDescriptorSet.dstArrayElement = 0;
		cameraWriteDescriptorSet.descriptorCount = 1;
		cameraWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraWriteDescriptorSet.pImageInfo = nullptr;
		cameraWriteDescriptorSet.pBufferInfo = &cameraInfo;
		cameraWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(cameraWriteDescriptorSet);

		VkWriteDescriptorSet skyboxWriteDescriptorSet = {};
		skyboxWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		skyboxWriteDescriptorSet.pNext = nullptr;
		skyboxWriteDescriptorSet.dstSet = skyboxDescriptorSets[i].descriptorSet;
		skyboxWriteDescriptorSet.dstBinding = 1;
		skyboxWriteDescriptorSet.dstArrayElement = 0;
		skyboxWriteDescriptorSet.descriptorCount = 1;
		skyboxWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		skyboxWriteDescriptorSet.pImageInfo = &skyboxInfo;
		skyboxWriteDescriptorSet.pBufferInfo = nullptr;
		skyboxWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(skyboxWriteDescriptorSet);

		skyboxDescriptorSets[i].update(writesDescriptorSet);
	}

	// Cleanup
	envmapImage.destroy();
}

void Envmap::destroy() {
	skyboxGraphicsPipeline.destroy();
	cubeVertexBuffer.destroy();
	cubeIndexBuffer.destroy();
	defaultSkybox.destroy();
	skyboxImage.destroy();
	diffuseIradianceImage.destroy();
	prefilterImage.destroy();
	brdfConvolutionImage.destroy();
}

void Envmap::draw(CommandBuffer* commandBuffer) {
	VkDeviceSize offset = 0;

	vkCmdBindVertexBuffers(commandBuffer->commandBuffer, 0, 1, &cubeVertexBuffer.buffer, &offset);
	vkCmdBindIndexBuffer(commandBuffer->commandBuffer, cubeIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer->commandBuffer, 36, 1, 0, 0, 0);
}

void Envmap::equiRectangleToCubemap() {
	Viewport equiRecToCubemapViewport;
	equiRecToCubemapViewport.init(ENVMAP_WIDTH, ENVMAP_HEIGHT);

	std::vector<RenderPassAttachment> attachments;
	attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, physicalDevice.colorFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

	std::vector<SubpassDependency> dependencies;
	
	RenderPass equiRecToCubemapRenderPass;
	equiRecToCubemapRenderPass.init(attachments, dependencies);

	std::array<Framebuffer, 6> equiRecToCubemapFramebuffers;
	std::array<VkImageView, 6> equiRecToCubemapImageViews;

	for (int face = 0; face < 6; face++) {
		ImageTools::createImageView(&equiRecToCubemapImageViews[face], skyboxImage.image, face, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		std::vector<VkImageView> framebufferAttachments;
		framebufferAttachments.push_back(equiRecToCubemapImageViews[face]);
		equiRecToCubemapFramebuffers[face].init(&equiRecToCubemapRenderPass, framebufferAttachments, ENVMAP_WIDTH, ENVMAP_HEIGHT, 1);
	}

	GraphicsPipeline equiRecToCubemapGraphicsPipeline;
	equiRecToCubemapGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/envmap/cubemap.vert";
	equiRecToCubemapGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/envmap/equiRecToCubemap.frag";
	equiRecToCubemapGraphicsPipeline.renderPass = &equiRecToCubemapRenderPass;
	equiRecToCubemapGraphicsPipeline.viewport = &equiRecToCubemapViewport;
	equiRecToCubemapGraphicsPipeline.multiSample = false;
	equiRecToCubemapGraphicsPipeline.init();
	
	DescriptorSet equiRecToCubemapDescriptorSet;
	equiRecToCubemapDescriptorSet.init(&equiRecToCubemapGraphicsPipeline, 0);

	VkDescriptorImageInfo skyboxInfo = {};
	skyboxInfo.sampler = trilinearEdgeBlackSampler;
	skyboxInfo.imageView = envmapImage.image != VK_NULL_HANDLE ? envmapImage.imageView : defaultSkybox.imageView;
	skyboxInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	std::vector<VkWriteDescriptorSet> writesDescriptorSet;

	VkWriteDescriptorSet skyboxWriteDescriptorSet = {};
	skyboxWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	skyboxWriteDescriptorSet.pNext = nullptr;
	skyboxWriteDescriptorSet.dstSet = equiRecToCubemapDescriptorSet.descriptorSet;
	skyboxWriteDescriptorSet.dstBinding = 0;
	skyboxWriteDescriptorSet.dstArrayElement = 0;
	skyboxWriteDescriptorSet.descriptorCount = 1;
	skyboxWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	skyboxWriteDescriptorSet.pImageInfo = &skyboxInfo;
	skyboxWriteDescriptorSet.pBufferInfo = nullptr;
	skyboxWriteDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(skyboxWriteDescriptorSet);

	equiRecToCubemapDescriptorSet.update(writesDescriptorSet);

	glm::mat4 projection = Camera::createPerspectiveProjection(90.0f, ENVMAP_WIDTH / static_cast<float>(ENVMAP_HEIGHT), 0.1f, 2.0f, true);
	std::array<glm::mat4, 6> views = { Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0)),
		Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0)),
		Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0)),
		Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0)),
		Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0)),
		Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0)),
	};

	for (int face = 0; face < 6; face++) {
		VkDeviceSize offset = 0;

		CommandPool commandPool;
		commandPool.init();
		CommandBuffer commandBuffer;
		commandBuffer.init(&commandPool);

		commandBuffer.begin();

		glm::mat4 viewProj = projection * views[face];

		equiRecToCubemapRenderPass.begin(&commandBuffer, equiRecToCubemapFramebuffers[face].framebuffer, { ENVMAP_WIDTH, ENVMAP_HEIGHT });

		equiRecToCubemapGraphicsPipeline.bind(&commandBuffer);
		equiRecToCubemapGraphicsPipeline.pushConstant(&commandBuffer, VK_SHADER_STAGE_VERTEX_BIT, 0, 16 * sizeof(float), &viewProj);
		equiRecToCubemapDescriptorSet.bind(&commandBuffer, &equiRecToCubemapGraphicsPipeline, 0);

		vkCmdBindVertexBuffers(commandBuffer.commandBuffer, 0, 1, &cubeVertexBuffer.buffer, &offset);
		vkCmdBindIndexBuffer(commandBuffer.commandBuffer, cubeIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffer.commandBuffer, 36, 1, 0, 0, 0);

		equiRecToCubemapRenderPass.end(&commandBuffer);

		commandBuffer.endAndSubmit();
		commandPool.destroy();
	}

	uint32_t skyboxMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(ENVMAP_WIDTH, ENVMAP_HEIGHT)))) + 1;
	ImageTools::transitionLayout(skyboxImage.image, physicalDevice.colorFormat, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, skyboxMipLevels, 6);
	ImageTools::generateMipmaps(skyboxImage.image, physicalDevice.colorFormat, ENVMAP_WIDTH, ENVMAP_HEIGHT, skyboxMipLevels, 6);

	equiRecToCubemapRenderPass.destroy();
	equiRecToCubemapGraphicsPipeline.destroy();
	for (int i = 0; i < 6; i++) {
		equiRecToCubemapFramebuffers[i].destroy();
		vkDestroyImageView(logicalDevice.device, equiRecToCubemapImageViews[i], nullptr);
	}
}

void Envmap::createDiffuseIradiance() {
	ImageTools::createImage(&diffuseIradianceImage.image, 6, CONVOLVE_WIDTH, CONVOLVE_HEIGHT, 1, VK_SAMPLE_COUNT_1_BIT, physicalDevice.colorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &diffuseIradianceImage.memoryInfo);
	ImageTools::createImageView(&diffuseIradianceImage.imageView, diffuseIradianceImage.image, 0, 6, 0, 1, VK_IMAGE_VIEW_TYPE_CUBE, physicalDevice.colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	ImageTools::transitionLayout(diffuseIradianceImage.image, physicalDevice.colorFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 6);

	Viewport convolveViewport;
	convolveViewport.init(CONVOLVE_WIDTH, CONVOLVE_HEIGHT);

	std::vector<RenderPassAttachment> attachments;
	attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, physicalDevice.colorFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

	std::vector<SubpassDependency> dependencies;

	RenderPass convolveRenderPass;
	convolveRenderPass.init(attachments, dependencies);

	std::array<Framebuffer, 6> diffuseIradianceFramebuffers;
	std::array<VkImageView, 6> diffuseIradianceImageViews;
	for (int face = 0; face < 6; face++) {
		ImageTools::createImageView(&diffuseIradianceImageViews[face], diffuseIradianceImage.image, face, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		std::vector<VkImageView> framebufferAttachments;
		framebufferAttachments.push_back(diffuseIradianceImageViews[face]);
		diffuseIradianceFramebuffers[face].init(&convolveRenderPass, framebufferAttachments, CONVOLVE_WIDTH, CONVOLVE_HEIGHT, 1);
	}

	GraphicsPipeline convolveGraphicsPipeline;
	convolveGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/envmap/cubemap.vert";
	convolveGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/envmap/convolve.frag";
	convolveGraphicsPipeline.renderPass = &convolveRenderPass;
	convolveGraphicsPipeline.viewport = &convolveViewport;
	convolveGraphicsPipeline.multiSample = false;
	convolveGraphicsPipeline.init();

	DescriptorSet convolveDescriptorSet;
	convolveDescriptorSet.init(&convolveGraphicsPipeline, 0);

	VkDescriptorImageInfo skyboxInfo = {};
	skyboxInfo.sampler = trilinearEdgeBlackSampler;
	skyboxInfo.imageView = skyboxImage.image != VK_NULL_HANDLE ? skyboxImage.imageView : defaultSkybox.imageView;
	skyboxInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	std::vector<VkWriteDescriptorSet> writesDescriptorSet;

	VkWriteDescriptorSet skyboxWriteDescriptorSet = {};
	skyboxWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	skyboxWriteDescriptorSet.pNext = nullptr;
	skyboxWriteDescriptorSet.dstSet = convolveDescriptorSet.descriptorSet;
	skyboxWriteDescriptorSet.dstBinding = 0;
	skyboxWriteDescriptorSet.dstArrayElement = 0;
	skyboxWriteDescriptorSet.descriptorCount = 1;
	skyboxWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	skyboxWriteDescriptorSet.pImageInfo = &skyboxInfo;
	skyboxWriteDescriptorSet.pBufferInfo = nullptr;
	skyboxWriteDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(skyboxWriteDescriptorSet);

	convolveDescriptorSet.update(writesDescriptorSet);

	glm::mat4 projection = Camera::createPerspectiveProjection(90.0f, CONVOLVE_WIDTH / static_cast<float>(CONVOLVE_HEIGHT), 0.1f, 2.0f, true);
	std::array<glm::mat4, 6> views = { Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0)),
		Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0)),
		Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0)),
		Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0)),
		Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0)),
		Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0)),
	};

	for (int face = 0; face < 6; face++) {
		VkDeviceSize offset = 0;

		CommandPool commandPool;
		commandPool.init();
		CommandBuffer commandBuffer;
		commandBuffer.init(&commandPool);

		commandBuffer.begin();

		glm::mat4 viewProj = projection * views[face];

		convolveRenderPass.begin(&commandBuffer, diffuseIradianceFramebuffers[face].framebuffer, { CONVOLVE_WIDTH, CONVOLVE_HEIGHT });

		convolveGraphicsPipeline.bind(&commandBuffer);
		convolveGraphicsPipeline.pushConstant(&commandBuffer, VK_SHADER_STAGE_VERTEX_BIT, 0, 16 * sizeof(float), &viewProj);
		convolveDescriptorSet.bind(&commandBuffer, &convolveGraphicsPipeline, 0);

		vkCmdBindVertexBuffers(commandBuffer.commandBuffer, 0, 1, &cubeVertexBuffer.buffer, &offset);
		vkCmdBindIndexBuffer(commandBuffer.commandBuffer, cubeIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffer.commandBuffer, 36, 1, 0, 0, 0);

		convolveRenderPass.end(&commandBuffer);

		commandBuffer.endAndSubmit();
		commandPool.destroy();
	}

	convolveRenderPass.destroy();
	convolveGraphicsPipeline.destroy();
	for (int i = 0; i < 6; i++) {
		diffuseIradianceFramebuffers[i].destroy();
		vkDestroyImageView(logicalDevice.device, diffuseIradianceImageViews[i], nullptr);
	}
}

void Envmap::createPrefilter() {
	ImageTools::createImage(&prefilterImage.image, 6, PREFILTER_WIDTH, PREFILTER_HEIGHT, 5, VK_SAMPLE_COUNT_1_BIT, physicalDevice.colorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prefilterImage.memoryInfo);
	ImageTools::createImageView(&prefilterImage.imageView, prefilterImage.image, 0, 6, 0, 5, VK_IMAGE_VIEW_TYPE_CUBE, physicalDevice.colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	ImageTools::transitionLayout(prefilterImage.image, physicalDevice.colorFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 5, 6);

	std::vector<RenderPassAttachment> attachments;
	attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, physicalDevice.colorFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

	std::vector<SubpassDependency> dependencies;

	RenderPass prefilterRenderPass;
	prefilterRenderPass.init(attachments, dependencies);

	GraphicsPipeline prefilterGraphicsPipeline;
	prefilterGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/envmap/cubemap.vert";
	prefilterGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/envmap/prefilter.frag";
	prefilterGraphicsPipeline.renderPass = &prefilterRenderPass;
	prefilterGraphicsPipeline.multiSample = false;

	std::array<Framebuffer, 30> prefilterFramebuffers;
	std::array<VkImageView, 30> prefilterImageViews;

	Buffer roughnessBuffer;
	BufferTools::createUniformBuffer(roughnessBuffer.buffer, sizeof(float), &roughnessBuffer.memoryInfo);

	for (int mipLevel = 0; mipLevel < 5; mipLevel++) {
		uint32_t mipWidth = static_cast<uint32_t>(PREFILTER_WIDTH * std::pow(0.5f, mipLevel));
		uint32_t mipHeight = static_cast<uint32_t>(PREFILTER_HEIGHT * std::pow(0.5f, mipLevel));

		Viewport prefilterViewport;
		prefilterViewport.init(mipWidth, mipHeight);

		for (int face = 0; face < 6; face++) {
			ImageTools::createImageView(&prefilterImageViews[mipLevel * 6 + face], prefilterImage.image, face, 1, mipLevel, 1, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
			std::vector<VkImageView> framebufferAttachments;
			framebufferAttachments.push_back(prefilterImageViews[mipLevel * 6 + face]);
			prefilterFramebuffers[mipLevel * 6 + face].init(&prefilterRenderPass, framebufferAttachments, mipWidth, mipHeight, 1);
		}

		float roughness = static_cast<float>(mipLevel) / 4.0f;

		memcpy(reinterpret_cast<void*>(reinterpret_cast<char*>(roughnessBuffer.memoryInfo.data) + roughnessBuffer.memoryInfo.offset), &roughness, sizeof(float));

		prefilterGraphicsPipeline.viewport = &prefilterViewport;
		prefilterGraphicsPipeline.init();

		DescriptorSet prefilterDescriptorSet;
		prefilterDescriptorSet.init(&prefilterGraphicsPipeline, 0);

		VkDescriptorImageInfo skyboxInfo = {};
		skyboxInfo.sampler = trilinearEdgeBlackSampler;
		skyboxInfo.imageView = skyboxImage.image != VK_NULL_HANDLE ? skyboxImage.imageView : defaultSkybox.imageView;
		skyboxInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorBufferInfo roughnessInfo = {};
		roughnessInfo.buffer = roughnessBuffer.buffer;
		roughnessInfo.offset = 0;
		roughnessInfo.range = sizeof(float);

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet skyboxWriteDescriptorSet = {};
		skyboxWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		skyboxWriteDescriptorSet.pNext = nullptr;
		skyboxWriteDescriptorSet.dstSet = prefilterDescriptorSet.descriptorSet;
		skyboxWriteDescriptorSet.dstBinding = 0;
		skyboxWriteDescriptorSet.dstArrayElement = 0;
		skyboxWriteDescriptorSet.descriptorCount = 1;
		skyboxWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		skyboxWriteDescriptorSet.pImageInfo = &skyboxInfo;
		skyboxWriteDescriptorSet.pBufferInfo = nullptr;
		skyboxWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(skyboxWriteDescriptorSet);

		VkWriteDescriptorSet roughnessWriteDescriptorSet = {};
		roughnessWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		roughnessWriteDescriptorSet.pNext = nullptr;
		roughnessWriteDescriptorSet.dstSet = prefilterDescriptorSet.descriptorSet;
		roughnessWriteDescriptorSet.dstBinding = 1;
		roughnessWriteDescriptorSet.dstArrayElement = 0;
		roughnessWriteDescriptorSet.descriptorCount = 1;
		roughnessWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		roughnessWriteDescriptorSet.pImageInfo = nullptr;
		roughnessWriteDescriptorSet.pBufferInfo = &roughnessInfo;
		roughnessWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(roughnessWriteDescriptorSet);

		prefilterDescriptorSet.update(writesDescriptorSet);

		glm::mat4 projection = Camera::createPerspectiveProjection(90.0f, mipWidth / static_cast<float>(mipHeight), 0.1f, 2.0f, true);
		std::array<glm::mat4, 6> views = { Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0)),
			Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0)),
			Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0)),
			Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0)),
			Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0)),
			Camera::createLookAtView(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0)),
		};

		for (int face = 0; face < 6; face++) {
			VkDeviceSize offset = 0;

			CommandPool commandPool;
			commandPool.init();
			CommandBuffer commandBuffer;
			commandBuffer.init(&commandPool);

			commandBuffer.begin();

			glm::mat4 viewProj = projection * views[face];

			prefilterRenderPass.begin(&commandBuffer, prefilterFramebuffers[mipLevel * 6 + face].framebuffer, { mipWidth, mipHeight });

			prefilterGraphicsPipeline.bind(&commandBuffer);
			prefilterGraphicsPipeline.pushConstant(&commandBuffer, VK_SHADER_STAGE_VERTEX_BIT, 0, 16 * sizeof(float), &viewProj);
			prefilterDescriptorSet.bind(&commandBuffer, &prefilterGraphicsPipeline, 0);

			vkCmdBindVertexBuffers(commandBuffer.commandBuffer, 0, 1, &cubeVertexBuffer.buffer, &offset);
			vkCmdBindIndexBuffer(commandBuffer.commandBuffer, cubeIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBuffer.commandBuffer, 36, 1, 0, 0, 0);

			prefilterRenderPass.end(&commandBuffer);

			commandBuffer.endAndSubmit();
			commandPool.destroy();
		}

		prefilterGraphicsPipeline.destroyPipeline();
	}

	prefilterRenderPass.destroy();
	prefilterGraphicsPipeline.destroy();
	for (int i = 0; i < 30; i++) {
		prefilterFramebuffers[i].destroy();
		vkDestroyImageView(logicalDevice.device, prefilterImageViews[i], nullptr);
	}
	roughnessBuffer.destroy();
}

void Envmap::createBRDFConvolution() {
	ImageTools::createImage(&brdfConvolutionImage.image, 1, BRDFCONVOLUTION_WIDTH, BRDFCONVOLUTION_HEIGHT, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &brdfConvolutionImage.memoryInfo);
	ImageTools::createImageView(&brdfConvolutionImage.imageView, brdfConvolutionImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
	ImageTools::transitionLayout(brdfConvolutionImage.image, VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 1);

	Viewport brdfConvolutionViewport;
	brdfConvolutionViewport.init(BRDFCONVOLUTION_WIDTH, BRDFCONVOLUTION_HEIGHT);

	std::vector<RenderPassAttachment> attachments;
	attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R32G32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

	std::vector<SubpassDependency> dependencies;

	RenderPass brdfConvolutionRenderPass;
	brdfConvolutionRenderPass.init(attachments, dependencies);

	Framebuffer brdfConvolutionFramebuffer;
	std::vector<VkImageView> framebufferAttachments;
	framebufferAttachments.push_back(brdfConvolutionImage.imageView);
	brdfConvolutionFramebuffer.init(&brdfConvolutionRenderPass, framebufferAttachments, BRDFCONVOLUTION_WIDTH, BRDFCONVOLUTION_HEIGHT, 1);

	GraphicsPipeline brdfConvolutionGraphicsPipeline;
	brdfConvolutionGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/general/fullscreenTriangle.vert";
	brdfConvolutionGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/envmap/brdfConvolution.frag";
	brdfConvolutionGraphicsPipeline.renderPass = &brdfConvolutionRenderPass;
	brdfConvolutionGraphicsPipeline.viewport = &brdfConvolutionViewport;
	brdfConvolutionGraphicsPipeline.multiSample = false;
	brdfConvolutionGraphicsPipeline.frontFaceCCW = false;
	brdfConvolutionGraphicsPipeline.init();

	VkDeviceSize offset = 0;

	CommandPool commandPool;
	commandPool.init();
	CommandBuffer commandBuffer;
	commandBuffer.init(&commandPool);

	commandBuffer.begin();

	brdfConvolutionRenderPass.begin(&commandBuffer, brdfConvolutionFramebuffer.framebuffer, { BRDFCONVOLUTION_WIDTH, BRDFCONVOLUTION_HEIGHT });

	brdfConvolutionGraphicsPipeline.bind(&commandBuffer);

	vkCmdDraw(commandBuffer.commandBuffer, 3, 1, 0, 0);

	brdfConvolutionRenderPass.end(&commandBuffer);

	commandBuffer.endAndSubmit();
	commandPool.destroy();

	brdfConvolutionRenderPass.destroy();
	brdfConvolutionGraphicsPipeline.destroy();
	brdfConvolutionFramebuffer.destroy();
}
