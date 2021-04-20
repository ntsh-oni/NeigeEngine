#include "Renderer.h"
#include "resources/RendererResources.h"
#include "resources/ShaderResources.h"
#include "../window/WindowResources.h"
#include "../inputs/Inputs.h"
#include "../ecs/components/Transform.h"
#include "../ecs/components/Light.h"
#include "../ecs/components/Camera.h"
#include "../ecs/components/Renderable.h"

extern ECS ecs;

void Renderer::init(const std::string applicationName) {
	NEIGE_INFO("Renderer init start.");

	// Instance
	instance.init(applicationName, VK_MAKE_VERSION(0, 0, 1), window.instanceExtensions());

	// Surface
	window.createSurface();

	// Pick physical device
	PhysicalDevicePicker::pick();

	// Logical device
	logicalDevice.init();

	// Swapchain
	swapchain.init(&swapchainSize);

	NEIGE_INFO("Max frames in flight : " + std::to_string(MAX_FRAMES_IN_FLIGHT));
	NEIGE_INFO("Actual frames in flight : " + std::to_string(framesInFlight));
	NEIGE_INFO("Swapchain size : " + std::to_string(swapchainSize));
	NEIGE_INFO("Swapchain format : " + NeigeVKTranslate::vkFormatToString(swapchain.surfaceFormat.format));
	NEIGE_INFO("Swapchain color space : " + NeigeVKTranslate::vkColorSpaceToString(swapchain.surfaceFormat.colorSpace));
	NEIGE_INFO("Present mode : " + NeigeVKTranslate::vkPresentModeToString(swapchain.presentMode));
	NEIGE_INFO("Color format : " + NeigeVKTranslate::vkFormatToString(physicalDevice.colorFormat));
	NEIGE_INFO("Depth format : " + NeigeVKTranslate::vkFormatToString(physicalDevice.depthFormat));

	// Viewports
	fullscreenViewport.init(window.extent.width, window.extent.height);

	// Render passes

	// Opaque scene
	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));
		attachments.push_back(RenderPassAttachment(AttachmentType::DEPTH, physicalDevice.depthFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, ClearDepthValue(1.0f, 0)));
		
		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });

		RenderPass renderPass;
		renderPass.init(attachments, dependencies);
		renderPasses.emplace("opaqueScene", renderPass);
	}

	// Blend scene
	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f)));
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(1.0f, 0.0f, 0.0f, 0.0f)));
		attachments.push_back(RenderPassAttachment(AttachmentType::DEPTH, physicalDevice.depthFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, ClearDepthValue(1.0f, 0)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });

		RenderPass renderPass;
		renderPass.init(attachments, dependencies);
		renderPasses.emplace("blendScene", renderPass);
	}

	// Alpha compositing
	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });

		RenderPass renderPass;
		renderPass.init(attachments, dependencies);
		renderPasses.emplace("alphaCompositing", renderPass);
	}

	// Post-process
	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, swapchain.surfaceFormat.format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });

		RenderPass renderPass;
		renderPass.init(attachments, dependencies);
		renderPasses.emplace("post", renderPass);
	}

	// Camera
	auto& cameraCamera = ecs.getComponent<Camera>(camera);
	cameraCamera.projection = Camera::createPerspectiveProjection(cameraCamera.FOV, window.extent.width / static_cast<float>(window.extent.height), cameraCamera.nearPlane, cameraCamera.farPlane, true);

	cameraBuffers.resize(framesInFlight);
	for (Buffer& buffer : cameraBuffers) {
		BufferTools::createUniformBuffer(buffer.buffer, sizeof(CameraUniformBufferObject), &buffer.memoryInfo);
	}

	// Lights
	lightingBuffers.resize(framesInFlight);
	for (Buffer& buffer : lightingBuffers) {
		BufferTools::createUniformBuffer(buffer.buffer, sizeof(LightingUniformBufferObject), &buffer.memoryInfo);
	}

	// Time
	timeBuffers.resize(framesInFlight);
	for (Buffer& buffer : timeBuffers) {
		BufferTools::createUniformBuffer(buffer.buffer, sizeof(double), &buffer.memoryInfo);
	}

	createBindlessDescriptorSet();
	
	// Create samplers
	ImageTools::createImageSampler(&trilinearEdgeBlackSampler, 1001, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VK_COMPARE_OP_ALWAYS);
	ImageTools::createImageSampler(&trilinearEdgeOneLodBlackSampler, 1, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VK_COMPARE_OP_ALWAYS);
	ImageTools::createImageSampler(&trilinearEdgeWhiteSampler, 1, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, VK_COMPARE_OP_ALWAYS);
	ImageTools::createImageSampler(&trilinearRepeatBlackSampler, 1001, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VK_COMPARE_OP_ALWAYS);
	ImageTools::createImageSampler(&nearestEdgeBlackSampler, 1001, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VK_COMPARE_OP_ALWAYS);
	ImageTools::createImageSampler(&nearestRepeatBlackSampler, 1001, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VK_COMPARE_OP_ALWAYS);

	// Depth prepass
	NEIGE_INFO("Depth prepass init start.");
	depthPrepass.init(fullscreenViewport);
	NEIGE_INFO("Depth prepass init end.");

	// Bloom
	NEIGE_INFO("Bloom init start.");
	bloom.init(fullscreenViewport);
	NEIGE_INFO("Bloom init end.");

	// SSAO
	NEIGE_INFO("SSAO init start.");
	ssao.init(fullscreenViewport);
	NEIGE_INFO("SSAO init end.");

	// Shadow
	NEIGE_INFO("Shadowmapping init start.");
	shadow.init();
	NEIGE_INFO("Shadowmapping init end.");

	// Envmap
	NEIGE_INFO("Environment map init start.");
	envmap.init(cameraCamera.envmapPath);
	NEIGE_INFO("Environment map init end.");

	skyboxGraphicsPipeline.vertexShaderPath = "../shaders/skybox.vert";
	skyboxGraphicsPipeline.fragmentShaderPath = "../shaders/skybox.frag";
	skyboxGraphicsPipeline.renderPass = &renderPasses.at("opaqueScene");
	skyboxGraphicsPipeline.multiSample = false;
	skyboxGraphicsPipeline.viewport = &fullscreenViewport;
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
		skyboxInfo.sampler = trilinearEdgeOneLodBlackSampler;
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

	// Image and famebuffers
	createResources();


	// FXAA
	NEIGE_INFO("FXAA init start.");
	fxaa.init(fullscreenViewport);
	NEIGE_INFO("FXAA init end.");

	// Shadow
	{
		for (Entity light : lights) {
			auto const& lightLight = ecs.getComponent<Light>(light);

			if (lightLight.type == LightType::DIRECTIONAL || lightLight.type == LightType::SPOT) {
				Framebuffer lightFramebuffer;

				Image depthAttachment;
				ImageTools::createImage(&depthAttachment.image, 1, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, 1, VK_SAMPLE_COUNT_1_BIT, physicalDevice.depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthAttachment.memoryInfo);
				ImageTools::createImageView(&depthAttachment.imageView, depthAttachment.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
				shadow.images.push_back(depthAttachment);

				std::vector<VkImageView> framebufferAttachments;
				framebufferAttachments.push_back(depthAttachment.imageView);
				lightFramebuffer.init(&shadow.renderPass, framebufferAttachments, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, 1);
				shadow.framebuffers.push_back(lightFramebuffer);

				shadow.mapCount++;
			}
		}
	}

	// Scene compositing
	GraphicsPipeline alphaCompositingGraphicsPipeline;
	alphaCompositingGraphicsPipeline.vertexShaderPath = "../shaders/fullscreenTriangle.vert";
	alphaCompositingGraphicsPipeline.fragmentShaderPath = "../shaders/alphaCompositing.frag";
	alphaCompositingGraphicsPipeline.renderPass = &renderPasses.at("alphaCompositing");
	alphaCompositingGraphicsPipeline.viewport = &fullscreenViewport;
	alphaCompositingGraphicsPipeline.multiSample = false;
	alphaCompositingGraphicsPipeline.depthCompare = Compare::LESS;
	alphaCompositingGraphicsPipeline.backfaceCulling = false;
	alphaCompositingGraphicsPipeline.blendings.push_back({ VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD });
	alphaCompositingGraphicsPipeline.init();
	graphicsPipelines.emplace("alphaCompositing", alphaCompositingGraphicsPipeline);

	createAlphaCompositingDescriptorSet();

	// Post-process
	GraphicsPipeline postGraphicsPipeline;
	postGraphicsPipeline.vertexShaderPath = "../shaders/fullscreenTriangle.vert";
	postGraphicsPipeline.fragmentShaderPath = "../shaders/postProcess.frag";
	postGraphicsPipeline.renderPass = &renderPasses.at("post");
	postGraphicsPipeline.viewport = &fullscreenViewport;
	postGraphicsPipeline.multiSample = false;
	postGraphicsPipeline.depthCompare = Compare::LESS;
	postGraphicsPipeline.backfaceCulling = false;
	postGraphicsPipeline.init();
	graphicsPipelines.emplace("post", postGraphicsPipeline);

	createPostProcessDescriptorSet();

	// Default textures
	float defaultDiffuse[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	Image defaultDiffuseImage;
	ImageTools::loadColor(defaultDiffuse, &defaultDiffuseImage.image, VK_FORMAT_R8G8B8A8_SRGB, &defaultDiffuseImage.mipmapLevels, &defaultDiffuseImage.memoryInfo);
	ImageTools::createImageView(&defaultDiffuseImage.imageView, defaultDiffuseImage.image, 0, 1, 0, defaultDiffuseImage.mipmapLevels, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	textures.push_back({ "defaultDiffuse", defaultDiffuseImage });

	float defaultNormal[4] = { 0.5f, 0.5f, 1.0f, 0.0f };
	Image defaultNormalImage;
	ImageTools::loadColor(defaultNormal, &defaultNormalImage.image, VK_FORMAT_R8G8B8A8_UNORM, &defaultNormalImage.mipmapLevels, &defaultNormalImage.memoryInfo);
	ImageTools::createImageView(&defaultNormalImage.imageView, defaultNormalImage.image, 0, 1, 0, defaultNormalImage.mipmapLevels, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	textures.push_back({ "defaultNormal", defaultNormalImage });

	float defaultMetallicRoughness[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	Image defaultMetallicRoughnessImage;
	ImageTools::loadColor(defaultMetallicRoughness, &defaultMetallicRoughnessImage.image, VK_FORMAT_R8G8B8A8_UNORM, &defaultMetallicRoughnessImage.mipmapLevels, &defaultMetallicRoughnessImage.memoryInfo);
	ImageTools::createImageView(&defaultMetallicRoughnessImage.imageView, defaultMetallicRoughnessImage.image, 0, 1, 0, defaultMetallicRoughnessImage.mipmapLevels, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	textures.push_back({ "defaultMetallicRoughness", defaultMetallicRoughnessImage });

	float defaultEmissive[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	Image defaultEmissiveImage;
	ImageTools::loadColor(defaultEmissive, &defaultEmissiveImage.image, VK_FORMAT_R8G8B8A8_UNORM, &defaultEmissiveImage.mipmapLevels, &defaultEmissiveImage.memoryInfo);
	ImageTools::createImageView(&defaultEmissiveImage.imageView, defaultEmissiveImage.image, 0, 1, 0, defaultEmissiveImage.mipmapLevels, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	textures.push_back({ "defaultEmissive", defaultEmissiveImage });

	float defaultOcclusion[4] = { 1.0, 1.0, 1.0, 0.0 };
	Image defaultOcclusionImage;
	ImageTools::loadColor(defaultOcclusion, &defaultOcclusionImage.image, VK_FORMAT_R8G8B8A8_UNORM, &defaultOcclusionImage.mipmapLevels, &defaultOcclusionImage.memoryInfo);
	ImageTools::createImageView(&defaultOcclusionImage.imageView, defaultOcclusionImage.image, 0, 1, 0, defaultOcclusionImage.mipmapLevels, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	textures.push_back({ "defaultOcclusion", defaultOcclusionImage });

	// Object resources
	for (Entity object : entities) {
		loadObject(object);
	}

	updateBindlessDescriptorSet();

	// Command pools and buffers
	renderingCommandPools.resize(framesInFlight);
	renderingCommandBuffers.resize(framesInFlight);
	for (uint32_t i = 0; i < framesInFlight; i++) {
		renderingCommandPools[i].init();
		renderingCommandBuffers[i].init(&renderingCommandPools[i]);
	}

	// Sync objects
	fences.resize(framesInFlight);
	IAsemaphores.resize(framesInFlight);
	for (uint32_t i = 0; i < framesInFlight; i++) {
		fences[i].init();
		IAsemaphores[i].init();
	}

	RFsemaphores.resize(swapchainSize);
	for (uint32_t i = 0; i < swapchainSize; i++) {
		RFsemaphores[i].init();
	}

	NEIGE_INFO("Renderer init end.");
}

void Renderer::update() {
	if (NEIGE_DEBUG) {
		if (keyboardInputs.pKey == KeyState::PRESSED) {
			logicalDevice.wait();
			NEIGE_INFO("Shaders reloading.");
			for (std::unordered_map<std::string, Shader>::iterator it = shaders.begin(); it != shaders.end(); it++) {
				it->second.reload();
			}
			for (std::unordered_map<std::string, GraphicsPipeline>::iterator it = graphicsPipelines.begin(); it != graphicsPipelines.end(); it++) {
				GraphicsPipeline* graphicsPipeline = &it->second;
				graphicsPipeline->destroyPipeline();
				graphicsPipeline->init();
			}
			skyboxGraphicsPipeline.destroyPipeline();
			skyboxGraphicsPipeline.init();
			ssao.depthToPositionsGraphicsPipeline.destroyPipeline();
			ssao.depthToPositionsGraphicsPipeline.init();
			ssao.depthToNormalsGraphicsPipeline.destroyPipeline();
			ssao.depthToNormalsGraphicsPipeline.init();
			ssao.ssaoGraphicsPipeline.destroyPipeline();
			ssao.ssaoGraphicsPipeline.init();
			ssao.ssaoBlurredGraphicsPipeline.destroyPipeline();
			ssao.ssaoBlurredGraphicsPipeline.init();
			bloom.blurGraphicsPipeline.destroyPipeline();
			bloom.blurGraphicsPipeline.init();
			fxaa.graphicsPipeline.destroyPipeline();
			fxaa.graphicsPipeline.init();
		}

		if (keyboardInputs.cKey == KeyState::PRESSED) {
			memoryAllocator.memoryAnalyzer();
		}
	}

	if (keyboardInputs.fKey == KeyState::PRESSED) {
		window.toggleFullscreen();
	}

	if (window.gotResized) {
		window.gotResized = false;
		reloadOnResize();
	}

	fences[currentFrame].wait();

	uint32_t swapchainImage;
	VkResult result = swapchain.acquireNextImage(&IAsemaphores[currentFrame], &swapchainImage);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		reloadOnResize();
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		NEIGE_ERROR("Unable to acquire swapchain image.");
	}

	updateData(currentFrame);

	recordRenderingCommands(currentFrame, swapchainImage);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &IAsemaphores[currentFrame].semaphore;
	VkPipelineStageFlags stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitDstStageMask = stages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &renderingCommandBuffers[currentFrame].commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &RFsemaphores[swapchainImage].semaphore;

	fences[currentFrame].reset();
	NEIGE_VK_CHECK(vkQueueSubmit(logicalDevice.queues.graphicsQueue, 1, &submitInfo, fences[currentFrame].fence));

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &RFsemaphores[swapchainImage].semaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain.swapchain;
	presentInfo.pImageIndices = &swapchainImage;
	presentInfo.pResults = nullptr;
	result = vkQueuePresentKHR(logicalDevice.queues.presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.gotResized) {
		window.gotResized = false;
		reloadOnResize();
	}
	else if (result != VK_SUCCESS) {
		NEIGE_ERROR("Unable to present image.");
	}

	currentFrame = (currentFrame + 1) % framesInFlight;
}

void Renderer::destroy() {
	logicalDevice.wait();
	destroyResources();
	if (trilinearEdgeBlackSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, trilinearEdgeBlackSampler, nullptr);
		trilinearEdgeBlackSampler = VK_NULL_HANDLE;
	}
	if (trilinearEdgeOneLodBlackSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, trilinearEdgeOneLodBlackSampler, nullptr);
		trilinearEdgeOneLodBlackSampler = VK_NULL_HANDLE;
	}
	if (trilinearEdgeWhiteSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, trilinearEdgeWhiteSampler, nullptr);
		trilinearEdgeWhiteSampler = VK_NULL_HANDLE;
	}
	if (trilinearRepeatBlackSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, trilinearRepeatBlackSampler, nullptr);
		trilinearRepeatBlackSampler = VK_NULL_HANDLE;
	}
	if (nearestEdgeBlackSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, nearestEdgeBlackSampler, nullptr);
		nearestEdgeBlackSampler = VK_NULL_HANDLE;
	}
	if (nearestRepeatBlackSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, nearestRepeatBlackSampler, nullptr);
		nearestRepeatBlackSampler = VK_NULL_HANDLE;
	}
	depthPrepass.destroy();
	envmap.destroy();
	shadow.destroy();
	fxaa.destroy();
	ssao.destroy();
	bloom.destroy();
	if (texturesDescriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(logicalDevice.device, texturesDescriptorPool, nullptr);
		texturesDescriptorPool = VK_NULL_HANDLE;
	}
	if (texturesDescriptorSetLayout != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(logicalDevice.device, texturesDescriptorSetLayout, nullptr);
		texturesDescriptorSetLayout = VK_NULL_HANDLE;
	}
	for (CommandPool& renderingCommandPool : renderingCommandPools) {
		renderingCommandPool.destroy();
	}
	for (std::unordered_map<std::string, RenderPass>::iterator it = renderPasses.begin(); it != renderPasses.end(); it++) {
		RenderPass* renderPass = &it->second;
		renderPass->destroy();
	}
	for (Buffer& buffer : cameraBuffers) {
		buffer.destroy();
	}
	for (Buffer& buffer : lightingBuffers) {
		buffer.destroy();
	}
	for (Buffer& buffer : timeBuffers) {
		buffer.destroy();
	}
	for (Entity entity : entities) {
		auto& objectRenderable = ecs.getComponent<Renderable>(entity);

		for (Buffer& buffer : objectRenderable.buffers) {
			buffer.destroy();
		}
	}
	for (std::unordered_map<std::string, GraphicsPipeline>::iterator it = graphicsPipelines.begin(); it != graphicsPipelines.end(); it++) {
		GraphicsPipeline* graphicsPipeline = &it->second;
		graphicsPipeline->destroy();
	}
	skyboxGraphicsPipeline.destroy();
	for (std::unordered_map<std::string, Shader>::iterator it = shaders.begin(); it != shaders.end(); it++) {
		Shader* shader = &it->second;
		shader->destroy();
	}
	for (size_t i = 0; i < textures.size(); i++) {
		textures[i].image.destroy();
	}
	for (std::unordered_map<std::string, Model>::iterator it = models.begin(); it != models.end(); it++) {
		Model* model = &it->second;
		model->destroy();
	}
	for (Fence& fence : fences) {
		fence.destroy();
	}
	for (Semaphore& IAsemaphore : IAsemaphores) {
		IAsemaphore.destroy();
	}
	for (Semaphore& RFsemaphore : RFsemaphores) {
		RFsemaphore.destroy();
	}
	memoryAllocator.destroy();
	window.surface.destroy();
	logicalDevice.destroy();
	instance.destroy();
}

void Renderer::loadObject(Entity object) {
	auto& objectRenderable = ecs.getComponent<Renderable>(object);

	objectRenderable.createLookupString();

	// Model
	if (models.find(objectRenderable.modelPath) == models.end()) {
		Model model;
		model.init(objectRenderable.modelPath);
		models.emplace(objectRenderable.modelPath, model);
	}
	Model* model = &models.at(objectRenderable.modelPath);

	// Graphics pipelines
	// Opaque
	if (model->gotOpaquePrimitives && graphicsPipelines.find(objectRenderable.lookupString + "o") == graphicsPipelines.end()) {
		GraphicsPipeline opaqueGraphicsPipeline;
		opaqueGraphicsPipeline.vertexShaderPath = objectRenderable.vertexShaderPath;
		opaqueGraphicsPipeline.fragmentShaderPath = objectRenderable.fragmentShaderPath;
		opaqueGraphicsPipeline.tesselationControlShaderPath = objectRenderable.tesselationControlShaderPath;
		opaqueGraphicsPipeline.tesselationEvaluationShaderPath = objectRenderable.tesselationEvaluationShaderPath;
		opaqueGraphicsPipeline.geometryShaderPath = objectRenderable.geometryShaderPath;
		opaqueGraphicsPipeline.renderPass = &renderPasses.at("opaqueScene");
		opaqueGraphicsPipeline.multiSample = false;
		opaqueGraphicsPipeline.viewport = &fullscreenViewport;
		opaqueGraphicsPipeline.topology = objectRenderable.topology;
		opaqueGraphicsPipeline.depthCompare = Compare::EQUAL;
		opaqueGraphicsPipeline.depthWrite = false;
		opaqueGraphicsPipeline.init();
		graphicsPipelines.emplace(objectRenderable.lookupString + "o", opaqueGraphicsPipeline);
	}

	// Mask
	std::string maskFragmentShader = objectRenderable.fragmentShaderPath.substr(0, objectRenderable.fragmentShaderPath.length() - 5) + "_m.frag";
	if (model->gotMaskPrimitives && graphicsPipelines.find(objectRenderable.lookupString + "m") == graphicsPipelines.end() && FileTools::exists(maskFragmentShader)) {
		GraphicsPipeline maskGraphicsPipeline;
		maskGraphicsPipeline.vertexShaderPath = objectRenderable.vertexShaderPath;
		maskGraphicsPipeline.fragmentShaderPath = maskFragmentShader;
		maskGraphicsPipeline.tesselationControlShaderPath = objectRenderable.tesselationControlShaderPath;
		maskGraphicsPipeline.tesselationEvaluationShaderPath = objectRenderable.tesselationEvaluationShaderPath;
		maskGraphicsPipeline.geometryShaderPath = objectRenderable.geometryShaderPath;
		maskGraphicsPipeline.renderPass = &renderPasses.at("opaqueScene");
		maskGraphicsPipeline.multiSample = false;
		maskGraphicsPipeline.viewport = &fullscreenViewport;
		maskGraphicsPipeline.topology = objectRenderable.topology;
		maskGraphicsPipeline.depthCompare = Compare::EQUAL;
		maskGraphicsPipeline.depthWrite = false;
		maskGraphicsPipeline.backfaceCulling = false;
		maskGraphicsPipeline.init();
		graphicsPipelines.emplace(objectRenderable.lookupString + "m", maskGraphicsPipeline);
	}

	// Blend
	std::string blendFragmentShader = objectRenderable.fragmentShaderPath.substr(0, objectRenderable.fragmentShaderPath.length() - 5) + "_b.frag";
	if (model->gotBlendPrimitives && graphicsPipelines.find(objectRenderable.lookupString + "b") == graphicsPipelines.end() && FileTools::exists(blendFragmentShader)) {
		GraphicsPipeline blendGraphicsPipeline;
		blendGraphicsPipeline.vertexShaderPath = objectRenderable.vertexShaderPath;
		blendGraphicsPipeline.fragmentShaderPath = blendFragmentShader;
		blendGraphicsPipeline.tesselationControlShaderPath = objectRenderable.tesselationControlShaderPath;
		blendGraphicsPipeline.tesselationEvaluationShaderPath = objectRenderable.tesselationEvaluationShaderPath;
		blendGraphicsPipeline.geometryShaderPath = objectRenderable.geometryShaderPath;
		blendGraphicsPipeline.renderPass = &renderPasses.at("blendScene");
		blendGraphicsPipeline.multiSample = false;
		blendGraphicsPipeline.viewport = &fullscreenViewport;
		blendGraphicsPipeline.topology = objectRenderable.topology;
		blendGraphicsPipeline.depthCompare = Compare::LESS_OR_EQUAL;
		blendGraphicsPipeline.depthWrite = false;
		blendGraphicsPipeline.backfaceCulling = false;
		blendGraphicsPipeline.blendings.push_back({ VK_TRUE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD });
		blendGraphicsPipeline.blendings.push_back({ VK_TRUE, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD });
		blendGraphicsPipeline.init();
		graphicsPipelines.emplace(objectRenderable.lookupString + "b", blendGraphicsPipeline);
	}

	objectRenderable.opaqueGraphicsPipeline = (graphicsPipelines.find(objectRenderable.lookupString + "o") != graphicsPipelines.end()) ? &graphicsPipelines.at(objectRenderable.lookupString + "o") : nullptr;;
	objectRenderable.maskGraphicsPipeline = (graphicsPipelines.find(objectRenderable.lookupString + "m") != graphicsPipelines.end()) ? &graphicsPipelines.at(objectRenderable.lookupString + "m") : nullptr;
	objectRenderable.blendGraphicsPipeline = (graphicsPipelines.find(objectRenderable.lookupString + "b") != graphicsPipelines.end()) ? &graphicsPipelines.at(objectRenderable.lookupString + "b") : nullptr;

	objectRenderable.buffers.resize(framesInFlight);
	objectRenderable.descriptorSets.resize(framesInFlight);
	objectRenderable.depthPrepassDescriptorSets.resize(framesInFlight);
	objectRenderable.depthPrepassMaskDescriptorSets.resize(framesInFlight);
	objectRenderable.shadowDescriptorSets.resize(framesInFlight);
	objectRenderable.shadowMaskDescriptorSets.resize(framesInFlight);

	// Buffers
	for (uint32_t i = 0; i < framesInFlight; i++) {
		BufferTools::createUniformBuffer(objectRenderable.buffers.at(i).buffer, sizeof(ObjectUniformBufferObject), &objectRenderable.buffers.at(i).memoryInfo);
	}

	if ((model->gotOpaquePrimitives && objectRenderable.opaqueGraphicsPipeline->sets.size() != 0) || (model->gotMaskPrimitives && objectRenderable.maskGraphicsPipeline->sets.size() != 0) || (model->gotBlendPrimitives && objectRenderable.blendGraphicsPipeline->sets.size() != 0)) {
		for (uint32_t i = 0; i < framesInFlight; i++) {
			// Descriptor sets
			GraphicsPipeline* graphicsPipeline = objectRenderable.opaqueGraphicsPipeline ? objectRenderable.opaqueGraphicsPipeline : (objectRenderable.maskGraphicsPipeline ? objectRenderable.maskGraphicsPipeline : objectRenderable.blendGraphicsPipeline);
			objectRenderable.createEntityDescriptorSet(i, graphicsPipeline);
		}
	}

	if (model->gotOpaquePrimitives && objectRenderable.opaqueGraphicsPipeline->sets.size() != 0) {
		for (uint32_t i = 0; i < framesInFlight; i++) {
			// Shadow
			objectRenderable.createShadowEntityDescriptorSet(i);

			// Depth prepass
			objectRenderable.createDepthPrepassEntityDescriptorSet(i);
		}
	}

	if (model->gotMaskPrimitives && objectRenderable.maskGraphicsPipeline->sets.size() != 0) {
		for (uint32_t i = 0; i < framesInFlight; i++) {

			// Depth prepass mask
			objectRenderable.createDepthPrepassMaskEntityDescriptorSet(i);

			// Shadow mask
			objectRenderable.createShadowMaskEntityDescriptorSet(i);
		}
	}
}

void Renderer::updateData(uint32_t frameInFlightIndex) {
	void* data;

	// Camera
	auto& cameraCamera = ecs.getComponent<Camera>(camera);

	CameraUniformBufferObject cubo = {};
	cameraCamera.view = Camera::createLookAtView(cameraCamera.position, cameraCamera.position + cameraCamera.to, glm::vec3(0.0f, 1.0f, 0.0f));
	cubo.view = cameraCamera.view;
	cubo.projection = cameraCamera.projection;
	cubo.position = cameraCamera.position;

	cameraBuffers.at(frameInFlightIndex).map(0, sizeof(CameraUniformBufferObject), &data);
	memcpy(data, &cubo, sizeof(CameraUniformBufferObject));
	cameraBuffers.at(frameInFlightIndex).unmap();

	// Lights
	int dirLightCount = 0;
	int pointLightCount = 0;
	int spotLightCount = 0;
	LightingUniformBufferObject lubo = {};
	ShadowUniformBufferObject subo = {};
	for (Entity entity : lights) {
		auto const& lightLight = ecs.getComponent<Light>(entity);

		if (lightLight.type == LightType::DIRECTIONAL) {
			lubo.dirLightsDirection[dirLightCount] = glm::vec4(lightLight.direction, 0.0f);
			lubo.dirLightsColor[dirLightCount] = glm::vec4(lightLight.color, 0.0f);

			glm::vec3 eye = -lightLight.direction;
			glm::vec3 up = glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), eye) == (glm::length(glm::vec3(0.0f, 1.0f, 0.0f)) * glm::length(eye)) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0);
			glm::mat4 shadowProjection = Camera::createOrthoProjection(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 20.0f);
			glm::mat4 shadowView = Camera::createLookAtView(eye + cameraCamera.position + (cameraCamera.to * 3.0f), glm::vec3(0.0f, 0.0f, 0.0f) + cameraCamera.position + (cameraCamera.to * 3.0f), up);
			subo.dirLightSpaces[dirLightCount] = shadowProjection * shadowView;

			dirLightCount++;
		}
		else if (lightLight.type == LightType::POINT) {
			lubo.pointLightsPosition[pointLightCount] = glm::vec4(lightLight.position, 0.0f);
			lubo.pointLightsColor[pointLightCount] = glm::vec4(lightLight.color, 0.0f);

			pointLightCount++;
		}
		else if (lightLight.type == LightType::SPOT) {
			lubo.spotLightsPosition[spotLightCount] = glm::vec4(lightLight.position, 0.0f);
			lubo.spotLightsDirection[spotLightCount] = glm::vec4(lightLight.direction, 0.0f);
			lubo.spotLightsColor[spotLightCount] = glm::vec4(lightLight.color, 0.0f);
			lubo.spotLightsCutoffs[spotLightCount] = glm::vec4(glm::cos(glm::radians(lightLight.cutoffs.x)), glm::cos(glm::radians(lightLight.cutoffs.y)), 0.0f, 0.0f);

			glm::vec3 eye = lightLight.position;
			glm::vec3 to = lightLight.direction;
			glm::vec3 up = glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), -to) == (glm::length(glm::vec3(0.0f, 1.0f, 0.0f)) * glm::length(-to)) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0);
			glm::mat4 shadowProjection = Camera::createPerspectiveProjection(120.0f, SHADOWMAP_WIDTH / static_cast<float>(SHADOWMAP_HEIGHT), 0.1f, 20.0f, false);
			glm::mat4 shadowView = Camera::createLookAtView(eye, eye + to, up);
			subo.spotLightSpaces[spotLightCount] = shadowProjection * shadowView;

			spotLightCount++;
		}
	}

	lubo.numLights.x = static_cast<float>(dirLightCount);
	lubo.numLights.y = static_cast<float>(pointLightCount);
	lubo.numLights.z = static_cast<float>(spotLightCount);

	subo.numLights.x = static_cast<float>(dirLightCount);
	subo.numLights.y = static_cast<float>(pointLightCount);
	subo.numLights.z = static_cast<float>(spotLightCount);

	lightingBuffers.at(frameInFlightIndex).map(0, sizeof(LightingUniformBufferObject), &data);
	memcpy(data, &lubo, sizeof(LightingUniformBufferObject));
	lightingBuffers.at(frameInFlightIndex).unmap();

	shadow.buffers.at(frameInFlightIndex).map(0, sizeof(ShadowUniformBufferObject), &data);
	memcpy(data, &subo, sizeof(ShadowUniformBufferObject));
	shadow.buffers.at(frameInFlightIndex).unmap();

	// Time
	float time = static_cast<float>(glfwGetTime());
	timeBuffers.at(frameInFlightIndex).map(0, sizeof(float), &data);
	memcpy(data, &time, sizeof(double));
	timeBuffers.at(frameInFlightIndex).unmap();

	// Renderables
	for (Entity object : entities) {
		auto const& objectTransform = ecs.getComponent<Transform>(object);
		auto& objectRenderable = ecs.getComponent<Renderable>(object);

		ObjectUniformBufferObject oubo = {};
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), objectTransform.position);
		glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), glm::radians(objectTransform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), glm::radians(objectTransform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f), glm::radians(objectTransform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), objectTransform.scale);
		oubo.model = translate * rotateX * rotateY * rotateZ * scale;

		objectRenderable.buffers.at(frameInFlightIndex).map(0, sizeof(ObjectUniformBufferObject), &data);
		memcpy(data, &oubo, sizeof(ObjectUniformBufferObject));
		objectRenderable.buffers.at(frameInFlightIndex).unmap();
	}
}

void Renderer::recordRenderingCommands(uint32_t frameInFlightIndex, uint32_t framebufferIndex) {
	RenderPass* opaqueSceneRenderPass = &renderPasses.at("opaqueScene");
	RenderPass* blendSceneRenderPass = &renderPasses.at("blendScene");
	RenderPass* alphaCompositingRenderPass = &renderPasses.at("alphaCompositing");
	RenderPass* postRenderPass = &renderPasses.at("post");

	renderingCommandPools[frameInFlightIndex].reset();
	renderingCommandBuffers[frameInFlightIndex].begin();

	// Depth prepass
	depthPrepass.renderPass.begin(&renderingCommandBuffers[frameInFlightIndex], depthPrepass.framebuffer.framebuffer, window.extent);

	for (Entity object : entities) {
		auto& objectRenderable = ecs.getComponent<Renderable>(object);
		Model* model = &models.at(objectRenderable.modelPath);
		model->bindBuffers(&renderingCommandBuffers[frameInFlightIndex]);

		// Opaque
		if (model->gotOpaquePrimitives) {
			depthPrepass.opaqueGraphicsPipeline.bind(&renderingCommandBuffers[frameInFlightIndex]);
			objectRenderable.depthPrepassDescriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], 0);
			model->drawOpaque(&renderingCommandBuffers[frameInFlightIndex], &depthPrepass.opaqueGraphicsPipeline, false);
		}

		// Mask
		if (model->gotMaskPrimitives) {
			depthPrepass.maskGraphicsPipeline.bind(&renderingCommandBuffers[frameInFlightIndex]);
			objectRenderable.depthPrepassMaskDescriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], 0);
			model->drawMask(&renderingCommandBuffers[frameInFlightIndex], &depthPrepass.maskGraphicsPipeline, true, 0);
		}
	}

	depthPrepass.renderPass.end(&renderingCommandBuffers[frameInFlightIndex]);

	// Shadow
	int lightIndex = 0;
	for (Entity light : lights) {
		auto const& lightLight = ecs.getComponent<Light>(light);

		if (lightLight.type == LightType::DIRECTIONAL || lightLight.type == LightType::SPOT) {
			shadow.renderPass.begin(&renderingCommandBuffers[frameInFlightIndex], shadow.framebuffers[lightIndex].framebuffer, { SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT });

			for (Entity object : entities) {
				auto& objectRenderable = ecs.getComponent<Renderable>(object);
				Model* model = &models.at(objectRenderable.modelPath);
				model->bindBuffers(&renderingCommandBuffers[frameInFlightIndex]);

				// Opaque
				if (model->gotOpaquePrimitives) {
					shadow.opaqueGraphicsPipeline.bind(&renderingCommandBuffers[frameInFlightIndex]);
					objectRenderable.shadowDescriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], 0);
					shadow.opaqueGraphicsPipeline.pushConstant(&renderingCommandBuffers[frameInFlightIndex], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(int), &lightIndex);
					model->drawOpaque(&renderingCommandBuffers[frameInFlightIndex], &shadow.opaqueGraphicsPipeline, false);
				}

				// Mask
				if (model->gotMaskPrimitives) {
					shadow.maskGraphicsPipeline.bind(&renderingCommandBuffers[frameInFlightIndex]);
					objectRenderable.shadowMaskDescriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], 0);
					shadow.maskGraphicsPipeline.pushConstant(&renderingCommandBuffers[frameInFlightIndex], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(int), &lightIndex);
					model->drawMask(&renderingCommandBuffers[frameInFlightIndex], &shadow.maskGraphicsPipeline, true, 16);
				}
			}

			shadow.renderPass.end(&renderingCommandBuffers[frameInFlightIndex]);

			lightIndex++;
		}
	}

	// Opaque scene
	opaqueSceneRenderPass->begin(&renderingCommandBuffers[frameInFlightIndex], opaqueSceneFramebuffer.framebuffer, window.extent);
	for (Entity object : entities) {
		auto& objectRenderable = ecs.getComponent<Renderable>(object);
		Model* model = &models.at(objectRenderable.modelPath);
		model->bindBuffers(&renderingCommandBuffers[frameInFlightIndex]);

		// Opaque
		if (model->gotOpaquePrimitives) {
			objectRenderable.opaqueGraphicsPipeline->bind(&renderingCommandBuffers[frameInFlightIndex]);
			if (objectRenderable.opaqueGraphicsPipeline->sets.size() != 0) {
				objectRenderable.descriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], 0);
			}
			model->drawOpaque(&renderingCommandBuffers[frameInFlightIndex], objectRenderable.opaqueGraphicsPipeline, true);
		}

		// Mask
		if (model->gotMaskPrimitives) {
			objectRenderable.maskGraphicsPipeline->bind(&renderingCommandBuffers[frameInFlightIndex]);
			if (objectRenderable.maskGraphicsPipeline->sets.size() != 0) {
				objectRenderable.descriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], 0);
			}
			model->drawMask(&renderingCommandBuffers[frameInFlightIndex], objectRenderable.maskGraphicsPipeline, true, 0);
		}
	}
	skyboxGraphicsPipeline.bind(&renderingCommandBuffers[frameInFlightIndex]);
	skyboxDescriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], 0);

	envmap.draw(&renderingCommandBuffers[frameInFlightIndex]);

	opaqueSceneRenderPass->end(&renderingCommandBuffers[frameInFlightIndex]);

	// Blend scene
	blendSceneRenderPass->begin(&renderingCommandBuffers[frameInFlightIndex], blendSceneFramebuffer.framebuffer, window.extent);
	for (Entity object : entities) {
		auto& objectRenderable = ecs.getComponent<Renderable>(object);
		Model* model = &models.at(objectRenderable.modelPath);
		model->bindBuffers(&renderingCommandBuffers[frameInFlightIndex]);

		if (model->gotBlendPrimitives) {
			objectRenderable.blendGraphicsPipeline->bind(&renderingCommandBuffers[frameInFlightIndex]);
			if (objectRenderable.blendGraphicsPipeline->sets.size() != 0) {
				objectRenderable.descriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], 0);
			}
			model->drawBlend(&renderingCommandBuffers[frameInFlightIndex], objectRenderable.blendGraphicsPipeline, true);
		}
	}

	blendSceneRenderPass->end(&renderingCommandBuffers[frameInFlightIndex]);

	// Alpha compositing
	alphaCompositingRenderPass->begin(&renderingCommandBuffers[frameInFlightIndex], alphaCompositingFramebuffer.framebuffer, window.extent);
	graphicsPipelines.at("alphaCompositing").bind(&renderingCommandBuffers[frameInFlightIndex]);
	alphaCompositingDescriptorSet.bind(&renderingCommandBuffers[frameInFlightIndex], 0);
	
	vkCmdDraw(renderingCommandBuffers[frameInFlightIndex].commandBuffer, 3, 1, 0, 0);

	alphaCompositingRenderPass->end(&renderingCommandBuffers[frameInFlightIndex]);

	// Bloom
	bloom.draw(&renderingCommandBuffers[frameInFlightIndex]);

	// SSAO
	ssao.draw(&renderingCommandBuffers[frameInFlightIndex], frameInFlightIndex);

	// Post-processing
	postRenderPass->begin(&renderingCommandBuffers[frameInFlightIndex], postFramebuffer.framebuffer, window.extent);
	graphicsPipelines.at("post").bind(&renderingCommandBuffers[frameInFlightIndex]);
	postDescriptorSet.bind(&renderingCommandBuffers[frameInFlightIndex], 0);

	vkCmdDraw(renderingCommandBuffers[frameInFlightIndex].commandBuffer, 3, 1, 0, 0);

	postRenderPass->end(&renderingCommandBuffers[frameInFlightIndex]);

	// FXAA
	fxaa.draw(&renderingCommandBuffers[frameInFlightIndex], framebufferIndex);

	renderingCommandBuffers[frameInFlightIndex].end();
}

void Renderer::createResources() {
	// Opaque scene
	{
		ImageTools::createImage(&sceneImage.image, 1, window.extent.width, window.extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &sceneImage.memoryInfo);
		ImageTools::createImageView(&sceneImage.imageView, sceneImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
	
		std::vector<VkImageView> framebufferAttachments;
		framebufferAttachments.push_back(sceneImage.imageView);
		framebufferAttachments.push_back(bloom.thresholdImage.imageView);
		framebufferAttachments.push_back(depthPrepass.image.imageView);
		opaqueSceneFramebuffer.init(&renderPasses.at("opaqueScene"), framebufferAttachments, window.extent.width, window.extent.height, 1);
	}

	// Blend scene
	{
		ImageTools::createImage(&blendAccumulationImage.image, 1, window.extent.width, window.extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &blendAccumulationImage.memoryInfo);
		ImageTools::createImageView(&blendAccumulationImage.imageView, blendAccumulationImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

		ImageTools::createImage(&blendRevealageImage.image, 1, window.extent.width, window.extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &blendRevealageImage.memoryInfo);
		ImageTools::createImageView(&blendRevealageImage.imageView, blendRevealageImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

		std::vector<VkImageView> framebufferAttachments;
		framebufferAttachments.push_back(blendAccumulationImage.imageView);
		framebufferAttachments.push_back(blendRevealageImage.imageView);
		framebufferAttachments.push_back(depthPrepass.image.imageView);
		blendSceneFramebuffer.init(&renderPasses.at("blendScene"), framebufferAttachments, window.extent.width, window.extent.height, 1);
	}

	// Alpha compositing
	{
		std::vector<VkImageView> framebufferAttachments;
		framebufferAttachments.push_back(sceneImage.imageView);
		alphaCompositingFramebuffer.init(&renderPasses.at("alphaCompositing"), framebufferAttachments, window.extent.width, window.extent.height, 1);
	}

	// Post-process
	{
		ImageTools::createImage(&postProcessImage.image, 1, window.extent.width, window.extent.height, 1, VK_SAMPLE_COUNT_1_BIT, swapchain.surfaceFormat.format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &postProcessImage.memoryInfo);
		ImageTools::createImageView(&postProcessImage.imageView, postProcessImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, swapchain.surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);

		std::vector<VkImageView> framebufferAttachments;
		framebufferAttachments.push_back(postProcessImage.imageView);
		postFramebuffer.init(&renderPasses.at("post"), framebufferAttachments, window.extent.width, window.extent.height, 1);
	}
}

void Renderer::destroyResources() {
	swapchain.destroy();
	sceneImage.destroy();
	blendAccumulationImage.destroy();
	blendRevealageImage.destroy();
	postProcessImage.destroy();
	opaqueSceneFramebuffer.destroy();
	blendSceneFramebuffer.destroy();
	alphaCompositingFramebuffer.destroy();
	postFramebuffer.destroy();
}

void Renderer::createBindlessDescriptorSet() {
	texturesDescriptorSet.init();
}

void Renderer::updateBindlessDescriptorSet() {
	std::vector<VkDescriptorImageInfo> textureInfos;
	for (Texture& texture : textures) {
		VkDescriptorImageInfo textureInfo = {};
		textureInfo.sampler = trilinearRepeatBlackSampler;
		textureInfo.imageView = texture.image.imageView;
		textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		textureInfos.push_back(textureInfo);
	}

	std::vector<VkWriteDescriptorSet> writesDescriptorSet;

	VkWriteDescriptorSet textureWriteDescriptorSet = {};
	textureWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	textureWriteDescriptorSet.pNext = nullptr;
	textureWriteDescriptorSet.dstSet = texturesDescriptorSet.descriptorSet;
	textureWriteDescriptorSet.dstBinding = 0;
	textureWriteDescriptorSet.dstArrayElement = 0;
	textureWriteDescriptorSet.descriptorCount = static_cast<uint32_t>(textures.size());
	textureWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureWriteDescriptorSet.pImageInfo = textureInfos.data();
	textureWriteDescriptorSet.pBufferInfo = nullptr;
	textureWriteDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(textureWriteDescriptorSet);

	texturesDescriptorSet.update(writesDescriptorSet);
}

void Renderer::createAlphaCompositingDescriptorSet() {
	alphaCompositingDescriptorSet.init(&graphicsPipelines.at("alphaCompositing"), 0);

	VkDescriptorImageInfo accumulationInfo = {};
	accumulationInfo.sampler = trilinearEdgeBlackSampler;
	accumulationInfo.imageView = blendAccumulationImage.imageView;
	accumulationInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkDescriptorImageInfo revealageInfo = {};
	revealageInfo.sampler = trilinearEdgeBlackSampler;
	revealageInfo.imageView = blendRevealageImage.imageView;
	revealageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	std::vector<VkWriteDescriptorSet> writesDescriptorSet;

	VkWriteDescriptorSet accumulationWriteDescriptorSet = {};
	accumulationWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	accumulationWriteDescriptorSet.pNext = nullptr;
	accumulationWriteDescriptorSet.dstSet = alphaCompositingDescriptorSet.descriptorSet;
	accumulationWriteDescriptorSet.dstBinding = 0;
	accumulationWriteDescriptorSet.dstArrayElement = 0;
	accumulationWriteDescriptorSet.descriptorCount = 1;
	accumulationWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	accumulationWriteDescriptorSet.pImageInfo = &accumulationInfo;
	accumulationWriteDescriptorSet.pBufferInfo = nullptr;
	accumulationWriteDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(accumulationWriteDescriptorSet);

	VkWriteDescriptorSet revealageWriteDescriptorSet = {};
	revealageWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	revealageWriteDescriptorSet.pNext = nullptr;
	revealageWriteDescriptorSet.dstSet = alphaCompositingDescriptorSet.descriptorSet;
	revealageWriteDescriptorSet.dstBinding = 1;
	revealageWriteDescriptorSet.dstArrayElement = 0;
	revealageWriteDescriptorSet.descriptorCount = 1;
	revealageWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	revealageWriteDescriptorSet.pImageInfo = &revealageInfo;
	revealageWriteDescriptorSet.pBufferInfo = nullptr;
	revealageWriteDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(revealageWriteDescriptorSet);

	alphaCompositingDescriptorSet.update(writesDescriptorSet);
}

void Renderer::createPostProcessDescriptorSet() {
	postDescriptorSet.init(&graphicsPipelines.at("post"), 0);

	VkDescriptorImageInfo sceneInfo = {};
	sceneInfo.sampler = trilinearEdgeBlackSampler;
	sceneInfo.imageView = sceneImage.imageView;
	sceneInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkDescriptorImageInfo bloomInfo = {};
	bloomInfo.sampler = trilinearEdgeBlackSampler;
	bloomInfo.imageView = bloom.bloomImage.imageView;
	bloomInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkDescriptorImageInfo ssaoInfo = {};
	ssaoInfo.sampler = nearestEdgeBlackSampler;
	ssaoInfo.imageView = ssao.ssaoBlurredImage.imageView;
	ssaoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	std::vector<VkWriteDescriptorSet> writesDescriptorSet;

	VkWriteDescriptorSet sceneWriteDescriptorSet = {};
	sceneWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	sceneWriteDescriptorSet.pNext = nullptr;
	sceneWriteDescriptorSet.dstSet = postDescriptorSet.descriptorSet;
	sceneWriteDescriptorSet.dstBinding = 0;
	sceneWriteDescriptorSet.dstArrayElement = 0;
	sceneWriteDescriptorSet.descriptorCount = 1;
	sceneWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sceneWriteDescriptorSet.pImageInfo = &sceneInfo;
	sceneWriteDescriptorSet.pBufferInfo = nullptr;
	sceneWriteDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(sceneWriteDescriptorSet);

	VkWriteDescriptorSet bloomWriteDescriptorSet = {};
	bloomWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	bloomWriteDescriptorSet.pNext = nullptr;
	bloomWriteDescriptorSet.dstSet = postDescriptorSet.descriptorSet;
	bloomWriteDescriptorSet.dstBinding = 1;
	bloomWriteDescriptorSet.dstArrayElement = 0;
	bloomWriteDescriptorSet.descriptorCount = 1;
	bloomWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bloomWriteDescriptorSet.pImageInfo = &bloomInfo;
	bloomWriteDescriptorSet.pBufferInfo = nullptr;
	bloomWriteDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(bloomWriteDescriptorSet);

	VkWriteDescriptorSet ssaoWriteDescriptorSet = {};
	ssaoWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	ssaoWriteDescriptorSet.pNext = nullptr;
	ssaoWriteDescriptorSet.dstSet = postDescriptorSet.descriptorSet;
	ssaoWriteDescriptorSet.dstBinding = 2;
	ssaoWriteDescriptorSet.dstArrayElement = 0;
	ssaoWriteDescriptorSet.descriptorCount = 1;
	ssaoWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	ssaoWriteDescriptorSet.pImageInfo = &ssaoInfo;
	ssaoWriteDescriptorSet.pBufferInfo = nullptr;
	ssaoWriteDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(ssaoWriteDescriptorSet);

	postDescriptorSet.update(writesDescriptorSet);
}

void Renderer::reloadOnResize() {
	while (window.extent.width == 0 || window.extent.height == 0) {
		window.waitEvents();
	}
	logicalDevice.wait();

	destroyResources();

	// Swapchain
	swapchain.init(&swapchainSize);

	fullscreenViewport.viewport.width = static_cast<float>(window.extent.width);
	fullscreenViewport.viewport.height = static_cast<float>(window.extent.height);
	fullscreenViewport.scissor.extent.width = window.extent.width;
	fullscreenViewport.scissor.extent.height = window.extent.height;

	// Depth prepass
	depthPrepass.destroyResources();
	depthPrepass.createResources(fullscreenViewport);

	// Bloom
	bloom.destroyResources();
	bloom.createResources(fullscreenViewport);

	// SSAO
	ssao.destroyResources();
	ssao.createResources(fullscreenViewport);

	// Image and framebuffers
	createResources();

	// FXAA
	fxaa.destroyResources();
	fxaa.createResources(fullscreenViewport);

	createAlphaCompositingDescriptorSet();

	createPostProcessDescriptorSet();

	auto& cameraCamera = ecs.getComponent<Camera>(camera);
	cameraCamera.projection = Camera::createPerspectiveProjection(cameraCamera.FOV, window.extent.width / static_cast<float>(window.extent.height), cameraCamera.nearPlane, cameraCamera.farPlane, true);
}
