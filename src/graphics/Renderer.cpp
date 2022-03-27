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

void Renderer::init(const std::string& applicationName) {
	// Init internal systems
	cameraSystem->init();
	lighting->init();

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
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));
		attachments.push_back(RenderPassAttachment(AttachmentType::UNUSED, VK_FORMAT_R8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f)));
		attachments.push_back(RenderPassAttachment(AttachmentType::DEPTH, physicalDevice.depthFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, ClearDepthValue(1.0f, 0)));
		
		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });

		RenderPass renderPass;
		renderPass.init(attachments, dependencies);
		renderPasses.emplace("opaqueScene", renderPass);
	}

	// Blend scene
	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f)));
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, VK_FORMAT_R8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(1.0f, 0.0f, 0.0f, 0.0f)));
		attachments.push_back(RenderPassAttachment(AttachmentType::DEPTH, physicalDevice.depthFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, ClearDepthValue(1.0f, 0)));

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
		std::vector<SubpassDependency> dependencies;

		if (enableFXAA) {
			attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, swapchain.surfaceFormat.format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

			dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
			dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		}
		else {
			attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, swapchain.surfaceFormat.format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

			dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
			dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		}

		RenderPass renderPass;
		renderPass.init(attachments, dependencies);
		renderPasses.emplace("post", renderPass);
	}

	// Camera
	cameraBuffers.resize(framesInFlight);
	for (Buffer& buffer : cameraBuffers) {
		BufferTools::createUniformBuffer(buffer.buffer, sizeof(CameraUniformBufferObject), &buffer.memoryInfo);
	}

	frustumBuffers.resize(framesInFlight);
	for (Buffer& buffer : frustumBuffers) {
		BufferTools::createUniformBuffer(buffer.buffer, 6 * 4 * sizeof(float), &buffer.memoryInfo);
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

	// Default sprite image
	SpriteImage spriteImage;
	ImageTools::createImage(&spriteImage.image.image, 1, 1, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &spriteImage.image.memoryInfo);
	ImageTools::createImageView(&spriteImage.image.imageView, spriteImage.image.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	spritesImages.push_back(spriteImage);

	// Default font
	Font font;
	ImageTools::createImage(&font.image.image, 1, 1, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &font.image.memoryInfo);
	ImageTools::createImageView(&font.image.imageView, font.image.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	fonts.push_back(font);

	createAdditionalDescriptorSets();
	
	// Create samplers
	ImageTools::createImageSampler(&trilinearEdgeBlackSampler, 1001, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, 16.0f, VK_COMPARE_OP_ALWAYS);
	ImageTools::createImageSampler(&trilinearEdgeOneLodBlackSampler, 1, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, 16.0f, VK_COMPARE_OP_ALWAYS);
	ImageTools::createImageSampler(&trilinearOffscreenSampler, 1, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, 0.0f, VK_COMPARE_OP_ALWAYS);
	ImageTools::createImageSampler(&trilinearLodOffscreenSampler, 1001, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, 0.0f, VK_COMPARE_OP_ALWAYS);
	ImageTools::createImageSampler(&nearestOffscreenSampler, 1, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, 0.0f, VK_COMPARE_OP_ALWAYS);
	ImageTools::createImageSampler(&nearestRepeatOffscreenSampler, 1, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, 0.0f, VK_COMPARE_OP_ALWAYS);
	ImageTools::createImageSampler(&shadowSampler, 1, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, 0.0f, VK_COMPARE_OP_LESS);
	ImageTools::createImageSampler(&trilinearRepeatBlackSampler, 1001, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, 16.0f, VK_COMPARE_OP_ALWAYS);
	ImageTools::createImageSampler(&nearestEdgeBlackSampler, 1001, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, 16.0f, VK_COMPARE_OP_ALWAYS);
	ImageTools::createImageSampler(&nearestEdgeOneLodBlackSampler, 1, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, 16.0f, VK_COMPARE_OP_ALWAYS);
	ImageTools::createImageSampler(&nearestRepeatBlackSampler, 1001, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, 16.0f, VK_COMPARE_OP_ALWAYS);
	ImageTools::createImageSampler(&fontSampler, 1, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, 0.0f, VK_COMPARE_OP_ALWAYS);

	// Depth prepass
	NEIGE_INFO("Depth prepass init start.");
	depthPrepass.init(fullscreenViewport);
	NEIGE_INFO("Depth prepass init end.");

	// SSAO
	if (enableSSAO) {
		NEIGE_INFO("SSAO init start.");
		ssao.init(ssaoDownscale, fullscreenViewport);
		NEIGE_INFO("SSAO init end.");
	}

	NEIGE_INFO("Frustum culling init start.");
	frustumCulling.init();
	NEIGE_INFO("Frustum culling init end.");

	// Shadow
	NEIGE_INFO("Shadowmapping init start.");
	shadow.init();
	NEIGE_INFO("Shadowmapping init end.");

	// Envmap
	NEIGE_INFO("Environment map init start.");
	envmap.init(scene.envmapPath, &fullscreenViewport, &renderPasses.at("opaqueScene"));
	NEIGE_INFO("Environment map init end.");

	// Default revealage attachment
	ImageTools::loadValue(0.0f, &defaultRevealageAttachment.image, VK_FORMAT_R8_UNORM, &defaultRevealageAttachment.mipmapLevels, &defaultRevealageAttachment.memoryInfo);
	ImageTools::createImageView(&defaultRevealageAttachment.imageView, defaultRevealageAttachment.image, 0, 1, 0, defaultRevealageAttachment.mipmapLevels, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	// Image and famebuffers
	createResources();

	// Atmosphere
	if (scene.skyboxType == SkyboxType::ATMOSPHERE) {
		NEIGE_INFO("Atmosphere init start.");
		atmosphere.init(fullscreenViewport);
		NEIGE_INFO("Atmosphere init end.");
	}

	// Bloom
	if (enableBloom) {
		NEIGE_INFO("Bloom init start.");
		bloom.init(bloomDownscale, bloomThreshold, bloomBlurBigKernel, fullscreenViewport);
		NEIGE_INFO("Bloom init end.");
	}

	// UI
	NEIGE_INFO("UI init start.");
	ui.init(fullscreenViewport);
	NEIGE_INFO("UI init end.");

	// FXAA
	if (enableFXAA) {
		NEIGE_INFO("FXAA init start.");
		fxaa.init(fullscreenViewport);
		NEIGE_INFO("FXAA init end.");
	}

	// Shadow
	{
		int dirLightCount = 0;
		int spotLightCount = 0;
		for (Entity light : *lights) {
			auto& lightLight = ecs.getComponent<Light>(light);

			if (lightLight.component.type == LightType::DIRECTIONAL) {
				Framebuffer lightFramebuffer;

				Image depthAttachment;
				ImageTools::createImage(&depthAttachment.image, 1, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, 1, VK_SAMPLE_COUNT_1_BIT, physicalDevice.depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthAttachment.memoryInfo);
				ImageTools::createImageView(&depthAttachment.imageView, depthAttachment.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
				shadow.directionalImages.push_back(depthAttachment);

				std::vector<VkImageView> framebufferAttachments;
				framebufferAttachments.push_back(depthAttachment.imageView);
				lightFramebuffer.init(&shadow.renderPass, framebufferAttachments, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, 1);
				shadow.directionalFramebuffers.push_back(lightFramebuffer);

				lightLight.shadowMapIndex = dirLightCount + 1;
				lightLight.savedShadowMapIndex = lightLight.shadowMapIndex;
				dirLightCount++;
			}
			else if (lightLight.component.type == LightType::SPOT) {
				Framebuffer lightFramebuffer;

				Image depthAttachment;
				ImageTools::createImage(&depthAttachment.image, 1, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, 1, VK_SAMPLE_COUNT_1_BIT, physicalDevice.depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthAttachment.memoryInfo);
				ImageTools::createImageView(&depthAttachment.imageView, depthAttachment.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
				shadow.spotImages.push_back(depthAttachment);

				std::vector<VkImageView> framebufferAttachments;
				framebufferAttachments.push_back(depthAttachment.imageView);
				lightFramebuffer.init(&shadow.renderPass, framebufferAttachments, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, 1);
				shadow.spotFramebuffers.push_back(lightFramebuffer);

				lightLight.shadowMapIndex = spotLightCount + 1;
				lightLight.savedShadowMapIndex = lightLight.shadowMapIndex;
				spotLightCount++;
			}
		}
	}

	// Scene compositing
	GraphicsPipeline alphaCompositingGraphicsPipeline;
	alphaCompositingGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/general/fullscreenTriangle.vert";
	alphaCompositingGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/wboit/alphaCompositing.frag";
	alphaCompositingGraphicsPipeline.renderPass = &renderPasses.at("alphaCompositing");
	alphaCompositingGraphicsPipeline.viewport = &fullscreenViewport;
	alphaCompositingGraphicsPipeline.multiSample = false;
	alphaCompositingGraphicsPipeline.depthCompare = Compare::LESS;
	alphaCompositingGraphicsPipeline.frontFaceCCW = false;
	alphaCompositingGraphicsPipeline.blendings.push_back({ VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD });
	alphaCompositingGraphicsPipeline.init();
	graphicsPipelines.emplace("alphaCompositing", alphaCompositingGraphicsPipeline);

	createAlphaCompositingDescriptorSet();

	// Post-process
	float defaultPostProcessEffect[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
	ImageTools::loadColor(defaultPostProcessEffect, &defaultPostProcessEffectImage.image, VK_FORMAT_R8G8B8A8_SRGB, &defaultPostProcessEffectImage.mipmapLevels, &defaultPostProcessEffectImage.memoryInfo);
	ImageTools::createImageView(&defaultPostProcessEffectImage.imageView, defaultPostProcessEffectImage.image, 0, 1, 0, defaultPostProcessEffectImage.mipmapLevels, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	GraphicsPipeline postGraphicsPipeline;
	postGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/general/fullscreenTriangle.vert";
	postGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/postprocess/postProcess.frag";
	postGraphicsPipeline.renderPass = &renderPasses.at("post");
	postGraphicsPipeline.viewport = &fullscreenViewport;
	postGraphicsPipeline.multiSample = false;
	postGraphicsPipeline.depthCompare = Compare::LESS;
	postGraphicsPipeline.frontFaceCCW = false;
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
	ImageTools::loadColor(defaultEmissive, &defaultEmissiveImage.image, VK_FORMAT_R8G8B8A8_SRGB, &defaultEmissiveImage.mipmapLevels, &defaultEmissiveImage.memoryInfo);
	ImageTools::createImageView(&defaultEmissiveImage.imageView, defaultEmissiveImage.image, 0, 1, 0, defaultEmissiveImage.mipmapLevels, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	textures.push_back({ "defaultEmissive", defaultEmissiveImage });

	float defaultOcclusion[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	Image defaultOcclusionImage;
	ImageTools::loadColor(defaultOcclusion, &defaultOcclusionImage.image, VK_FORMAT_R8G8B8A8_UNORM, &defaultOcclusionImage.mipmapLevels, &defaultOcclusionImage.memoryInfo);
	ImageTools::createImageView(&defaultOcclusionImage.imageView, defaultOcclusionImage.image, 0, 1, 0, defaultOcclusionImage.mipmapLevels, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	textures.push_back({ "defaultOcclusion", defaultOcclusionImage });

	// Default material
	Material defaultMaterial = { 0, 1, 2, 3, 4 };
	BufferTools::createStorageBuffer(defaultMaterial.buffer.buffer, 5 * sizeof(int), &defaultMaterial.buffer.memoryInfo);
	int materialindices[5] = { defaultMaterial.diffuseIndex, defaultMaterial.normalIndex, defaultMaterial.metallicRoughnessIndex, defaultMaterial.emissiveIndex, defaultMaterial.occlusionIndex };
	memcpy(reinterpret_cast<void*>(reinterpret_cast<char*>(defaultMaterial.buffer.memoryInfo.data) + defaultMaterial.buffer.memoryInfo.offset), materialindices, 5 * sizeof(int));

	materials.push_back(defaultMaterial);

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
			envmap.skyboxGraphicsPipeline.destroyPipeline();
			envmap.skyboxGraphicsPipeline.init();
			if (enableSSAO) {
				ssao.depthToPositionsGraphicsPipeline.destroyPipeline();
				ssao.depthToPositionsGraphicsPipeline.init();
				ssao.depthToNormalsGraphicsPipeline.destroyPipeline();
				ssao.depthToNormalsGraphicsPipeline.init();
				ssao.ssaoGraphicsPipeline.destroyPipeline();
				ssao.ssaoGraphicsPipeline.init();
				ssao.ssaoBlurredGraphicsPipeline.destroyPipeline();
				ssao.ssaoBlurredGraphicsPipeline.init();
			}
			if (enableBloom) {
				bloom.blurGraphicsPipeline.destroyPipeline();
				bloom.blurGraphicsPipeline.init();
			}
			if (enableFXAA) {
				fxaa.graphicsPipeline.destroyPipeline();
				fxaa.graphicsPipeline.init();
			}
		}
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
	swapchain.destroy();
	if (trilinearEdgeBlackSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, trilinearEdgeBlackSampler, nullptr);
		trilinearEdgeBlackSampler = VK_NULL_HANDLE;
	}
	if (trilinearEdgeOneLodBlackSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, trilinearEdgeOneLodBlackSampler, nullptr);
		trilinearEdgeOneLodBlackSampler = VK_NULL_HANDLE;
	}
	if (trilinearOffscreenSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, trilinearOffscreenSampler, nullptr);
		trilinearOffscreenSampler = VK_NULL_HANDLE;
	}
	if (trilinearLodOffscreenSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, trilinearLodOffscreenSampler, nullptr);
		trilinearLodOffscreenSampler = VK_NULL_HANDLE;
	}
	if (nearestOffscreenSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, nearestOffscreenSampler, nullptr);
		nearestOffscreenSampler = VK_NULL_HANDLE;
	}
	if (nearestRepeatOffscreenSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, nearestRepeatOffscreenSampler, nullptr);
		nearestRepeatOffscreenSampler = VK_NULL_HANDLE;
	}
	if (shadowSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, shadowSampler, nullptr);
		shadowSampler = VK_NULL_HANDLE;
	}
	if (trilinearRepeatBlackSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, trilinearRepeatBlackSampler, nullptr);
		trilinearRepeatBlackSampler = VK_NULL_HANDLE;
	}
	if (nearestEdgeBlackSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, nearestEdgeBlackSampler, nullptr);
		nearestEdgeBlackSampler = VK_NULL_HANDLE;
	}
	if (nearestEdgeOneLodBlackSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, nearestEdgeOneLodBlackSampler, nullptr);
		nearestEdgeOneLodBlackSampler = VK_NULL_HANDLE;
	}
	if (nearestRepeatBlackSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, nearestRepeatBlackSampler, nullptr);
		nearestRepeatBlackSampler = VK_NULL_HANDLE;
	}
	if (fontSampler != VK_NULL_HANDLE) {
		vkDestroySampler(logicalDevice.device, fontSampler, nullptr);
		fontSampler = VK_NULL_HANDLE;
	}
	depthPrepass.destroy();
	if (scene.skyboxType == SkyboxType::ATMOSPHERE) {
		atmosphere.destroy();
	}
	envmap.destroy();
	shadow.destroy();
	ui.destroy();
	if (enableFXAA) {
		fxaa.destroy();
	}
	if (enableSSAO) {
		ssao.destroy();
	}
	if (enableBloom) {
		bloom.destroy();
	}
	frustumCulling.destroy();
	defaultPostProcessEffectImage.destroy();
	defaultRevealageAttachment.destroy();
	materialsDescriptorPool.destroy();
	if (materialsDescriptorSetLayout != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(logicalDevice.device, materialsDescriptorSetLayout, nullptr);
		materialsDescriptorSetLayout = VK_NULL_HANDLE;
	}
	perDrawDescriptorPool.destroy();
	if (perDrawDescriptorSetLayout != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(logicalDevice.device, perDrawDescriptorSetLayout, nullptr);
		perDrawDescriptorSetLayout = VK_NULL_HANDLE;
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
	for (Buffer& buffer : frustumBuffers) {
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

		if (objectRenderable.loaded) {
			for (uint32_t i = 0; i < framesInFlight; i++) {
				objectRenderable.buffers[i].destroy();
			}

			if (objectRenderable.model->gotOpaquePrimitives) {
				objectRenderable.opaqueCulledDrawIndirectBuffer.destroy();
				objectRenderable.opaqueCulledDrawCountBuffer.destroy();
				objectRenderable.opaqueCulledDrawIndirectInfoBuffer.destroy();
			}

			if (objectRenderable.model->gotMaskPrimitives) {
				objectRenderable.maskCulledDrawIndirectBuffer.destroy();
				objectRenderable.maskCulledDrawCountBuffer.destroy();
				objectRenderable.maskCulledDrawIndirectInfoBuffer.destroy();
			}

			if (objectRenderable.model->gotBlendPrimitives) {
				objectRenderable.blendCulledDrawIndirectBuffer.destroy();
				objectRenderable.blendCulledDrawCountBuffer.destroy();
				objectRenderable.blendCulledDrawIndirectInfoBuffer.destroy();
			}
		}
	}
	for (std::unordered_map<std::string, GraphicsPipeline>::iterator it = graphicsPipelines.begin(); it != graphicsPipelines.end(); it++) {
		GraphicsPipeline* graphicsPipeline = &it->second;
		graphicsPipeline->destroy();
	}
	for (std::unordered_map<std::string, Shader>::iterator it = shaders.begin(); it != shaders.end(); it++) {
		Shader* shader = &it->second;
		shader->destroy();
	}
	for (size_t i = 0; i < textures.size(); i++) {
		textures[i].image.destroy();
	}
	for (size_t i = 0; i < materials.size(); i++) {
		materials[i].buffer.destroy();
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
	if (models.find(objectRenderable.component.modelPath) == models.end()) {
		Model model;
		model.init(objectRenderable.component.modelPath);
		models.emplace(objectRenderable.component.modelPath, model);
	}
	objectRenderable.model = &models.at(objectRenderable.component.modelPath);

	// Graphics pipelines
	// Opaque
	if (objectRenderable.model->gotOpaquePrimitives && graphicsPipelines.find(objectRenderable.lookupString + "o") == graphicsPipelines.end()) {
		GraphicsPipeline opaqueGraphicsPipeline;
		opaqueGraphicsPipeline.vertexShaderPath = objectRenderable.component.vertexShaderPath;
		opaqueGraphicsPipeline.fragmentShaderPath = objectRenderable.component.fragmentShaderPath;
		opaqueGraphicsPipeline.tesselationControlShaderPath = objectRenderable.component.tesselationControlShaderPath;
		opaqueGraphicsPipeline.tesselationEvaluationShaderPath = objectRenderable.component.tesselationEvaluationShaderPath;
		opaqueGraphicsPipeline.geometryShaderPath = objectRenderable.component.geometryShaderPath;
		opaqueGraphicsPipeline.renderPass = &renderPasses.at("opaqueScene");
		opaqueGraphicsPipeline.multiSample = false;
		opaqueGraphicsPipeline.viewport = &fullscreenViewport;
		opaqueGraphicsPipeline.topology = objectRenderable.topology;
		opaqueGraphicsPipeline.depthCompare = Compare::EQUAL;
		opaqueGraphicsPipeline.depthWrite = false;
		opaqueGraphicsPipeline.externalSets.push_back(1);
		opaqueGraphicsPipeline.externalSets.push_back(2);
		opaqueGraphicsPipeline.externalDescriptorSetLayouts.push_back(materialsDescriptorSetLayout);
		opaqueGraphicsPipeline.externalDescriptorSetLayouts.push_back(perDrawDescriptorSetLayout);
		opaqueGraphicsPipeline.specializationConstantValues.push_back(0);
		opaqueGraphicsPipeline.init();
		graphicsPipelines.emplace(objectRenderable.lookupString + "o", opaqueGraphicsPipeline);
	}

	// Mask
	if (objectRenderable.model->gotMaskPrimitives && graphicsPipelines.find(objectRenderable.lookupString + "m") == graphicsPipelines.end()) {
		GraphicsPipeline maskGraphicsPipeline;
		maskGraphicsPipeline.vertexShaderPath = objectRenderable.component.vertexShaderPath;
		maskGraphicsPipeline.fragmentShaderPath = objectRenderable.component.fragmentShaderPath;
		maskGraphicsPipeline.tesselationControlShaderPath = objectRenderable.component.tesselationControlShaderPath;
		maskGraphicsPipeline.tesselationEvaluationShaderPath = objectRenderable.component.tesselationEvaluationShaderPath;
		maskGraphicsPipeline.geometryShaderPath = objectRenderable.component.geometryShaderPath;
		maskGraphicsPipeline.renderPass = &renderPasses.at("opaqueScene");
		maskGraphicsPipeline.multiSample = false;
		maskGraphicsPipeline.viewport = &fullscreenViewport;
		maskGraphicsPipeline.topology = objectRenderable.topology;
		maskGraphicsPipeline.depthCompare = Compare::EQUAL;
		maskGraphicsPipeline.depthWrite = false;
		maskGraphicsPipeline.backfaceCulling = false;
		maskGraphicsPipeline.externalSets.push_back(1);
		maskGraphicsPipeline.externalSets.push_back(2);
		maskGraphicsPipeline.externalDescriptorSetLayouts.push_back(materialsDescriptorSetLayout);
		maskGraphicsPipeline.externalDescriptorSetLayouts.push_back(perDrawDescriptorSetLayout);
		maskGraphicsPipeline.specializationConstantValues.push_back(1);
		maskGraphicsPipeline.init();
		graphicsPipelines.emplace(objectRenderable.lookupString + "m", maskGraphicsPipeline);
	}

	// Blend
	if (objectRenderable.model->gotBlendPrimitives && graphicsPipelines.find(objectRenderable.lookupString + "b") == graphicsPipelines.end()) {
		GraphicsPipeline blendGraphicsPipeline;
		blendGraphicsPipeline.vertexShaderPath = objectRenderable.component.vertexShaderPath;
		blendGraphicsPipeline.fragmentShaderPath = objectRenderable.component.fragmentShaderPath;
		blendGraphicsPipeline.tesselationControlShaderPath = objectRenderable.component.tesselationControlShaderPath;
		blendGraphicsPipeline.tesselationEvaluationShaderPath = objectRenderable.component.tesselationEvaluationShaderPath;
		blendGraphicsPipeline.geometryShaderPath = objectRenderable.component.geometryShaderPath;
		blendGraphicsPipeline.renderPass = &renderPasses.at("blendScene");
		blendGraphicsPipeline.multiSample = false;
		blendGraphicsPipeline.viewport = &fullscreenViewport;
		blendGraphicsPipeline.topology = objectRenderable.topology;
		blendGraphicsPipeline.depthCompare = Compare::LESS_OR_EQUAL;
		blendGraphicsPipeline.depthWrite = false;
		blendGraphicsPipeline.backfaceCulling = false;
		blendGraphicsPipeline.blendings.push_back({ VK_TRUE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD });
		blendGraphicsPipeline.blendings.push_back({ VK_TRUE, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD });
		blendGraphicsPipeline.externalSets.push_back(1);
		blendGraphicsPipeline.externalSets.push_back(2);
		blendGraphicsPipeline.externalDescriptorSetLayouts.push_back(materialsDescriptorSetLayout);
		blendGraphicsPipeline.externalDescriptorSetLayouts.push_back(perDrawDescriptorSetLayout);
		blendGraphicsPipeline.specializationConstantValues.push_back(2);
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

	if (objectRenderable.model->gotOpaquePrimitives || objectRenderable.model->gotMaskPrimitives || objectRenderable.model->gotBlendPrimitives) {
		for (uint32_t i = 0; i < framesInFlight; i++) {
			// Descriptor sets
			GraphicsPipeline* graphicsPipeline = objectRenderable.opaqueGraphicsPipeline ? objectRenderable.opaqueGraphicsPipeline : (objectRenderable.maskGraphicsPipeline ? objectRenderable.maskGraphicsPipeline : objectRenderable.blendGraphicsPipeline);
			objectRenderable.createEntityDescriptorSet(i, graphicsPipeline);
		}
	}

	if (objectRenderable.model->gotOpaquePrimitives) {
		BufferTools::createBuffer(objectRenderable.opaqueCulledDrawIndirectBuffer.buffer, objectRenderable.model->opaqueDrawCount * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &objectRenderable.opaqueCulledDrawIndirectBuffer.memoryInfo);
		BufferTools::createBuffer(objectRenderable.opaqueCulledDrawCountBuffer.buffer, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &objectRenderable.opaqueCulledDrawCountBuffer.memoryInfo);
		BufferTools::createStorageBuffer(objectRenderable.opaqueCulledDrawIndirectInfoBuffer.buffer, objectRenderable.model->opaqueDrawCount * sizeof(PerDraw), &objectRenderable.opaqueCulledDrawIndirectInfoBuffer.memoryInfo);

		objectRenderable.indirectBuffers.push_back(objectRenderable.opaqueCulledDrawIndirectBuffer.buffer);
		objectRenderable.drawCountBuffers.push_back(objectRenderable.opaqueCulledDrawCountBuffer.buffer);
		objectRenderable.perDrawBuffers.push_back(objectRenderable.opaqueCulledDrawIndirectInfoBuffer.buffer);

		// Descriptor set allocation
		VkDescriptorSetAllocateInfo perDrawCulledDescriptorSetAllocateInfo = {};
		perDrawCulledDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		perDrawCulledDescriptorSetAllocateInfo.pNext = nullptr;
		perDrawCulledDescriptorSetAllocateInfo.descriptorPool = perDrawDescriptorPool.descriptorPool;
		perDrawCulledDescriptorSetAllocateInfo.descriptorSetCount = 1;
		perDrawCulledDescriptorSetAllocateInfo.pSetLayouts = &perDrawDescriptorSetLayout;
		NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &perDrawCulledDescriptorSetAllocateInfo, &objectRenderable.opaqueCulledDrawIndirectInfoDescriptorSet.descriptorSet));

		objectRenderable.opaqueCulledDrawIndirectInfoDescriptorSet.descriptorPool = &perDrawDescriptorPool;

		VkDescriptorBufferInfo perCulledDrawInfo = {};
		perCulledDrawInfo.buffer = objectRenderable.opaqueCulledDrawIndirectInfoBuffer.buffer;
		perCulledDrawInfo.offset = 0;
		perCulledDrawInfo.range = objectRenderable.model->opaqueDrawCount * sizeof(PerDraw);

		std::vector<VkWriteDescriptorSet> perDrawCulledWritesDescriptorSet;

		VkWriteDescriptorSet perDrawCulledWriteDescriptorSet = {};
		perDrawCulledWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		perDrawCulledWriteDescriptorSet.pNext = nullptr;
		perDrawCulledWriteDescriptorSet.dstSet = objectRenderable.opaqueCulledDrawIndirectInfoDescriptorSet.descriptorSet;
		perDrawCulledWriteDescriptorSet.dstBinding = 0;
		perDrawCulledWriteDescriptorSet.dstArrayElement = 0;
		perDrawCulledWriteDescriptorSet.descriptorCount = 1;
		perDrawCulledWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		perDrawCulledWriteDescriptorSet.pImageInfo = nullptr;
		perDrawCulledWriteDescriptorSet.pBufferInfo = &perCulledDrawInfo;
		perDrawCulledWriteDescriptorSet.pTexelBufferView = nullptr;
		perDrawCulledWritesDescriptorSet.push_back(perDrawCulledWriteDescriptorSet);

		objectRenderable.opaqueCulledDrawIndirectInfoDescriptorSet.update(perDrawCulledWritesDescriptorSet);

		objectRenderable.opaqueFrustumCullingDescriptorSets.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++) {
			// Frustum culling
			objectRenderable.createOpaqueFrustumCullingEntityDescriptorSet(i);

			// Depth prepass
			objectRenderable.createDepthPrepassEntityDescriptorSet(i);

			// Shadow
			objectRenderable.createShadowEntityDescriptorSet(i);
		}
	}

	if (objectRenderable.model->gotMaskPrimitives) {
		BufferTools::createBuffer(objectRenderable.maskCulledDrawIndirectBuffer.buffer, objectRenderable.model->maskDrawCount * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &objectRenderable.maskCulledDrawIndirectBuffer.memoryInfo);
		BufferTools::createBuffer(objectRenderable.maskCulledDrawCountBuffer.buffer, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &objectRenderable.maskCulledDrawCountBuffer.memoryInfo);
		BufferTools::createStorageBuffer(objectRenderable.maskCulledDrawIndirectInfoBuffer.buffer, objectRenderable.model->maskDrawCount * sizeof(PerDraw), &objectRenderable.maskCulledDrawIndirectInfoBuffer.memoryInfo);

		objectRenderable.indirectBuffers.push_back(objectRenderable.maskCulledDrawIndirectBuffer.buffer);
		objectRenderable.drawCountBuffers.push_back(objectRenderable.maskCulledDrawCountBuffer.buffer);
		objectRenderable.perDrawBuffers.push_back(objectRenderable.maskCulledDrawIndirectInfoBuffer.buffer);

		// Descriptor set allocation
		VkDescriptorSetAllocateInfo perDrawCulledDescriptorSetAllocateInfo = {};
		perDrawCulledDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		perDrawCulledDescriptorSetAllocateInfo.pNext = nullptr;
		perDrawCulledDescriptorSetAllocateInfo.descriptorPool = perDrawDescriptorPool.descriptorPool;
		perDrawCulledDescriptorSetAllocateInfo.descriptorSetCount = 1;
		perDrawCulledDescriptorSetAllocateInfo.pSetLayouts = &perDrawDescriptorSetLayout;
		NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &perDrawCulledDescriptorSetAllocateInfo, &objectRenderable.maskCulledDrawIndirectInfoDescriptorSet.descriptorSet));

		objectRenderable.maskCulledDrawIndirectInfoDescriptorSet.descriptorPool = &perDrawDescriptorPool;

		VkDescriptorBufferInfo perCulledDrawInfo = {};
		perCulledDrawInfo.buffer = objectRenderable.maskCulledDrawIndirectInfoBuffer.buffer;
		perCulledDrawInfo.offset = 0;
		perCulledDrawInfo.range = objectRenderable.model->maskDrawCount * sizeof(PerDraw);

		std::vector<VkWriteDescriptorSet> perDrawCulledWritesDescriptorSet;

		VkWriteDescriptorSet perDrawCulledWriteDescriptorSet = {};
		perDrawCulledWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		perDrawCulledWriteDescriptorSet.pNext = nullptr;
		perDrawCulledWriteDescriptorSet.dstSet = objectRenderable.maskCulledDrawIndirectInfoDescriptorSet.descriptorSet;
		perDrawCulledWriteDescriptorSet.dstBinding = 0;
		perDrawCulledWriteDescriptorSet.dstArrayElement = 0;
		perDrawCulledWriteDescriptorSet.descriptorCount = 1;
		perDrawCulledWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		perDrawCulledWriteDescriptorSet.pImageInfo = nullptr;
		perDrawCulledWriteDescriptorSet.pBufferInfo = &perCulledDrawInfo;
		perDrawCulledWriteDescriptorSet.pTexelBufferView = nullptr;
		perDrawCulledWritesDescriptorSet.push_back(perDrawCulledWriteDescriptorSet);

		objectRenderable.maskCulledDrawIndirectInfoDescriptorSet.update(perDrawCulledWritesDescriptorSet);

		objectRenderable.maskFrustumCullingDescriptorSets.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++) {
			// Frustum culling
			objectRenderable.createMaskFrustumCullingEntityDescriptorSet(i);

			// Depth prepass mask
			objectRenderable.createDepthPrepassMaskEntityDescriptorSet(i);

			// Shadow mask
			objectRenderable.createShadowMaskEntityDescriptorSet(i);
		}
	}

	if (objectRenderable.model->gotBlendPrimitives) {
		BufferTools::createBuffer(objectRenderable.blendCulledDrawIndirectBuffer.buffer, objectRenderable.model->blendDrawCount * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &objectRenderable.blendCulledDrawIndirectBuffer.memoryInfo);
		BufferTools::createBuffer(objectRenderable.blendCulledDrawCountBuffer.buffer, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &objectRenderable.blendCulledDrawCountBuffer.memoryInfo);
		BufferTools::createStorageBuffer(objectRenderable.blendCulledDrawIndirectInfoBuffer.buffer, objectRenderable.model->blendDrawCount * sizeof(PerDraw), &objectRenderable.blendCulledDrawIndirectInfoBuffer.memoryInfo);

		objectRenderable.indirectBuffers.push_back(objectRenderable.blendCulledDrawIndirectBuffer.buffer);
		objectRenderable.drawCountBuffers.push_back(objectRenderable.blendCulledDrawCountBuffer.buffer);
		objectRenderable.perDrawBuffers.push_back(objectRenderable.blendCulledDrawIndirectInfoBuffer.buffer);

		// Descriptor set allocation
		VkDescriptorSetAllocateInfo perDrawCulledDescriptorSetAllocateInfo = {};
		perDrawCulledDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		perDrawCulledDescriptorSetAllocateInfo.pNext = nullptr;
		perDrawCulledDescriptorSetAllocateInfo.descriptorPool = perDrawDescriptorPool.descriptorPool;
		perDrawCulledDescriptorSetAllocateInfo.descriptorSetCount = 1;
		perDrawCulledDescriptorSetAllocateInfo.pSetLayouts = &perDrawDescriptorSetLayout;
		NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &perDrawCulledDescriptorSetAllocateInfo, &objectRenderable.blendCulledDrawIndirectInfoDescriptorSet.descriptorSet));

		objectRenderable.blendCulledDrawIndirectInfoDescriptorSet.descriptorPool = &perDrawDescriptorPool;

		VkDescriptorBufferInfo perCulledDrawInfo = {};
		perCulledDrawInfo.buffer = objectRenderable.blendCulledDrawIndirectInfoBuffer.buffer;
		perCulledDrawInfo.offset = 0;
		perCulledDrawInfo.range = objectRenderable.model->blendDrawCount * sizeof(PerDraw);

		std::vector<VkWriteDescriptorSet> perDrawCulledWritesDescriptorSet;

		VkWriteDescriptorSet perDrawCulledWriteDescriptorSet = {};
		perDrawCulledWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		perDrawCulledWriteDescriptorSet.pNext = nullptr;
		perDrawCulledWriteDescriptorSet.dstSet = objectRenderable.blendCulledDrawIndirectInfoDescriptorSet.descriptorSet;
		perDrawCulledWriteDescriptorSet.dstBinding = 0;
		perDrawCulledWriteDescriptorSet.dstArrayElement = 0;
		perDrawCulledWriteDescriptorSet.descriptorCount = 1;
		perDrawCulledWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		perDrawCulledWriteDescriptorSet.pImageInfo = nullptr;
		perDrawCulledWriteDescriptorSet.pBufferInfo = &perCulledDrawInfo;
		perDrawCulledWriteDescriptorSet.pTexelBufferView = nullptr;
		perDrawCulledWritesDescriptorSet.push_back(perDrawCulledWriteDescriptorSet);

		objectRenderable.blendCulledDrawIndirectInfoDescriptorSet.update(perDrawCulledWritesDescriptorSet);

		objectRenderable.blendFrustumCullingDescriptorSets.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++) {
			// Frustum culling
			objectRenderable.createBlendFrustumCullingEntityDescriptorSet(i);
		}
	}
}

void Renderer::updateData(uint32_t frameInFlightIndex) {
	// Camera
	auto& cameraCamera = ecs.getComponent<Camera>(mainCamera);
	auto& cameraTransform = ecs.getComponent<Transform>(mainCamera);

	CameraUniformBufferObject cubo = {};
	cameraCamera.view = Camera::createLookAtView(cameraTransform.component.position, cameraTransform.component.position + cameraTransform.component.rotation, glm::vec3(0.0f, 1.0f, 0.0f));
	cubo.view = cameraCamera.view;
	cubo.projection = cameraCamera.projection;
	cubo.position = cameraTransform.component.position;

	memcpy(reinterpret_cast<void*>(reinterpret_cast<char*>(cameraBuffers.at(frameInFlightIndex).memoryInfo.data) + cameraBuffers.at(frameInFlightIndex).memoryInfo.offset), &cubo, sizeof(CameraUniformBufferObject));

	// Update camera's frustum
	cameraCamera.frustum.calculateFrustum(cameraCamera.view, cameraCamera.projection);

	memcpy(reinterpret_cast<void*>(reinterpret_cast<char*>(frustumBuffers.at(frameInFlightIndex).memoryInfo.data) + frustumBuffers.at(frameInFlightIndex).memoryInfo.offset), cameraCamera.frustum.frustum.data(), 6 * 4 * sizeof(float));

	// Lights
	bool foundMainLight = false;
	int dirLightCount = 0;
	int pointLightCount = 0;
	int spotLightCount = 0;
	LightingUniformBufferObject lubo = {};
	ShadowUniformBufferObject subo = {};
	for (Entity entity : *lights) {
		auto const& lightLight = ecs.getComponent<Light>(entity);
		auto const& lightTransform = ecs.getComponent<Transform>(entity);

		if (lightLight.component.type == LightType::DIRECTIONAL) {
			lubo.dirLightsDirectionAndShadowIndex[dirLightCount] = glm::vec4(lightTransform.component.rotation, static_cast<float>(lightLight.shadowMapIndex));
			lubo.dirLightsColor[dirLightCount] = glm::vec4(lightLight.component.color, 0.0f);

			if (!foundMainLight) {
				mainDirectionalLightDirection[0] = lightTransform.component.rotation.x;
				mainDirectionalLightDirection[1] = lightTransform.component.rotation.y;
				mainDirectionalLightDirection[2] = lightTransform.component.rotation.z;
				foundMainLight = true;
			}

			glm::vec3 eye = -lightTransform.component.rotation;
			glm::vec3 up = glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), eye) == (glm::length(glm::vec3(0.0f, 1.0f, 0.0f)) * glm::length(eye)) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0);
			glm::mat4 shadowProjection = Camera::createOrthoProjection(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 50.0f);
			glm::mat4 shadowView = Camera::createLookAtView(eye + cameraTransform.component.position + (cameraTransform.component.rotation * 10.0f), cameraTransform.component.position + (cameraTransform.component.rotation * 10.0f), up);
			subo.dirLightSpaces[dirLightCount] = shadowProjection * shadowView;

			dirLightCount++;
		}
		else if (lightLight.component.type == LightType::POINT) {
			lubo.pointLightsPosition[pointLightCount] = glm::vec4(lightTransform.component.position, 0.0f);
			lubo.pointLightsColor[pointLightCount] = glm::vec4(lightLight.component.color, 0.0f);

			pointLightCount++;
		}
		else if (lightLight.component.type == LightType::SPOT) {
			lubo.spotLightsPositionAndShadowIndex[spotLightCount] = glm::vec4(lightTransform.component.position, static_cast<float>(lightLight.shadowMapIndex));
			lubo.spotLightsDirection[spotLightCount] = glm::vec4(lightTransform.component.rotation, 0.0f);
			lubo.spotLightsColor[spotLightCount] = glm::vec4(lightLight.component.color, 0.0f);
			lubo.spotLightsCutoffs[spotLightCount] = glm::vec4(glm::cos(glm::radians(lightLight.component.cutoffs.x)), glm::cos(glm::radians(lightLight.component.cutoffs.y)), 0.0f, 0.0f);

			glm::vec3 eye = lightTransform.component.position;
			glm::vec3 to = lightTransform.component.rotation;
			glm::vec3 up = glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), -to) == (glm::length(glm::vec3(0.0f, 1.0f, 0.0f)) * glm::length(-to)) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0);
			glm::mat4 shadowProjection = Camera::createPerspectiveProjection(120.0f, SHADOWMAP_WIDTH / static_cast<float>(SHADOWMAP_HEIGHT), 0.1f, 20.0f, false);
			glm::mat4 shadowView = Camera::createLookAtView(eye, eye + to, up);
			subo.spotLightSpaces[spotLightCount] = shadowProjection * shadowView;

			spotLightCount++;
		}
	}
	if (!foundMainLight) {
		// No directional light found
		mainDirectionalLightDirection[0] = 1.0f;
		mainDirectionalLightDirection[1] = 1.0f;
		mainDirectionalLightDirection[2] = 1.0f;
	}

	lubo.numLights.x = static_cast<float>(dirLightCount);
	lubo.numLights.y = static_cast<float>(pointLightCount);
	lubo.numLights.z = static_cast<float>(spotLightCount);

	subo.numLights.x = static_cast<float>(dirLightCount);
	subo.numLights.y = static_cast<float>(pointLightCount);
	subo.numLights.z = static_cast<float>(spotLightCount);

	memcpy(reinterpret_cast<void*>(reinterpret_cast<char*>(lightingBuffers.at(frameInFlightIndex).memoryInfo.data) + lightingBuffers.at(frameInFlightIndex).memoryInfo.offset), &lubo, sizeof(LightingUniformBufferObject));

	memcpy(reinterpret_cast<void*>(reinterpret_cast<char*>(shadow.buffers.at(frameInFlightIndex).memoryInfo.data) + shadow.buffers.at(frameInFlightIndex).memoryInfo.offset), &subo, sizeof(ShadowUniformBufferObject));

	// Time
	float time = static_cast<float>(glfwGetTime());
	memcpy(reinterpret_cast<void*>(reinterpret_cast<char*>(timeBuffers.at(frameInFlightIndex).memoryInfo.data) + timeBuffers.at(frameInFlightIndex).memoryInfo.offset), &time, sizeof(float));

	// Renderables
	// Load and destroy entities
	std::vector<Entity> entitiesToDestroy;
	for (Entity object : entities) {
		auto& objectRenderable = ecs.getComponent<Renderable>(object);

		if (!objectRenderable.loaded) {
			loadObject(object);
			for (uint32_t i = 0; i < framesInFlight; i++) {
				materialDescriptorSetUpToDate[i] = false;
			}
			objectRenderable.loaded = true;
		}

		if (objectRenderable.destroyed) {
			if (objectRenderable.destroyCounter++ > framesInFlight) {
				for (uint32_t i = 0; i < framesInFlight; i++) {
					objectRenderable.buffers[i].destroy();
				}

				if (objectRenderable.model->gotOpaquePrimitives) {
					objectRenderable.opaqueCulledDrawIndirectBuffer.destroy();
					objectRenderable.opaqueCulledDrawCountBuffer.destroy();
					objectRenderable.opaqueCulledDrawIndirectInfoBuffer.destroy();
				}

				if (objectRenderable.model->gotMaskPrimitives) {
					objectRenderable.maskCulledDrawIndirectBuffer.destroy();
					objectRenderable.maskCulledDrawCountBuffer.destroy();
					objectRenderable.maskCulledDrawIndirectInfoBuffer.destroy();
				}

				if (objectRenderable.model->gotBlendPrimitives) {
					objectRenderable.blendCulledDrawIndirectBuffer.destroy();
					objectRenderable.blendCulledDrawCountBuffer.destroy();
					objectRenderable.blendCulledDrawIndirectInfoBuffer.destroy();
				}

				entitiesToDestroy.push_back(object);
			}
		}
	}

	for (Entity object : entitiesToDestroy) {
		ecs.destroyEntity(object);
	}

	if (!materialDescriptorSetUpToDate[frameInFlightIndex]) {
		updateMaterialDescriptorSet(frameInFlightIndex);
		materialDescriptorSetUpToDate[frameInFlightIndex] = true;
	}

	// UI
	if (!spriteDescriptorSetUpToDate[frameInFlightIndex]) {
		ui.updateSpriteDescriptorSet(frameInFlightIndex);
		spriteDescriptorSetUpToDate[frameInFlightIndex] = true;
	}
	if (!fontDescriptorSetUpToDate[frameInFlightIndex]) {
		ui.updateFontDescriptorSet(frameInFlightIndex);
		fontDescriptorSetUpToDate[frameInFlightIndex] = true;
	}

	for (Entity object : entities) {
		auto& objectRenderable = ecs.getComponent<Renderable>(object);

		if (objectRenderable.loaded && !objectRenderable.destroyed) {
			auto const& objectTransform = ecs.getComponent<Transform>(object);
			auto& objectRenderable = ecs.getComponent<Renderable>(object);

			ObjectUniformBufferObject oubo = {};
			glm::mat4 translate = glm::translate(glm::mat4(1.0f), objectTransform.component.position);
			glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), glm::radians(objectTransform.component.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), glm::radians(objectTransform.component.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f), glm::radians(objectTransform.component.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), objectTransform.component.scale);
			oubo.model = translate * rotateX * rotateY * rotateZ * scale;

			memcpy(reinterpret_cast<void*>(reinterpret_cast<char*>(objectRenderable.buffers.at(frameInFlightIndex).memoryInfo.data) + objectRenderable.buffers.at(frameInFlightIndex).memoryInfo.offset), &oubo, sizeof(ObjectUniformBufferObject));
		}
	}
}

void Renderer::recordRenderingCommands(uint32_t frameInFlightIndex, uint32_t framebufferIndex) {
	RenderPass* opaqueSceneRenderPass = &renderPasses.at("opaqueScene");
	RenderPass* blendSceneRenderPass = &renderPasses.at("blendScene");
	RenderPass* alphaCompositingRenderPass = &renderPasses.at("alphaCompositing");
	RenderPass* postRenderPass = &renderPasses.at("post");

	renderingCommandPools[frameInFlightIndex].reset();
	renderingCommandBuffers[frameInFlightIndex].begin();

	// Frustum culling
	std::vector<VkBuffer> indirectBuffers;
	std::vector<VkBuffer> drawCountBuffers;
	std::vector<VkBuffer> perDrawBuffers;
	std::vector<uint32_t> drawCounts;
	
	// Reset draw count and barriers
	for (Entity object : entities) {
		auto const& objectRenderable = ecs.getComponent<Renderable>(object);

		if (objectRenderable.loaded && !objectRenderable.destroyed) {
			indirectBuffers.insert(indirectBuffers.end(), objectRenderable.indirectBuffers.begin(), objectRenderable.indirectBuffers.end());
			drawCountBuffers.insert(drawCountBuffers.end(), objectRenderable.drawCountBuffers.begin(), objectRenderable.drawCountBuffers.end());
			perDrawBuffers.insert(perDrawBuffers.end(), objectRenderable.perDrawBuffers.begin(), objectRenderable.perDrawBuffers.end());
		}
	}

	frustumCulling.drawCountsReset(&renderingCommandBuffers[frameInFlightIndex], drawCountBuffers);

	frustumCulling.computePipeline.bind(&renderingCommandBuffers[frameInFlightIndex]);

	for (Entity object : entities) {
		auto& objectRenderable = ecs.getComponent<Renderable>(object);

		if (objectRenderable.loaded && !objectRenderable.destroyed) {
			// Opaque
			if (objectRenderable.model->gotOpaquePrimitives) {
				objectRenderable.opaqueFrustumCullingDescriptorSets[frameInFlightIndex].bind(&renderingCommandBuffers[frameInFlightIndex], &frustumCulling.computePipeline, 0);
				objectRenderable.cullOpaque(&renderingCommandBuffers[frameInFlightIndex], frameInFlightIndex);
				drawCounts.push_back(objectRenderable.model->opaqueDrawCount);
			}

			// Mask
			if (objectRenderable.model->gotMaskPrimitives) {
				objectRenderable.maskFrustumCullingDescriptorSets[frameInFlightIndex].bind(&renderingCommandBuffers[frameInFlightIndex], &frustumCulling.computePipeline, 0);
				objectRenderable.cullMask(&renderingCommandBuffers[frameInFlightIndex], frameInFlightIndex);
				drawCounts.push_back(objectRenderable.model->maskDrawCount);
			}

			// Blend
			if (objectRenderable.model->gotBlendPrimitives) {
				objectRenderable.blendFrustumCullingDescriptorSets[frameInFlightIndex].bind(&renderingCommandBuffers[frameInFlightIndex], &frustumCulling.computePipeline, 0);
				objectRenderable.cullBlend(&renderingCommandBuffers[frameInFlightIndex], frameInFlightIndex);
				drawCounts.push_back(objectRenderable.model->blendDrawCount);
			}
		}
	}

	frustumCulling.computeIndirectFragmentBarrier(&renderingCommandBuffers[frameInFlightIndex], indirectBuffers, drawCountBuffers, perDrawBuffers, drawCounts);

	// Depth prepass
	depthPrepass.renderPass.begin(&renderingCommandBuffers[frameInFlightIndex], depthPrepass.framebuffer.framebuffer, window.extent);

	for (Entity object : entities) {
		auto& objectRenderable = ecs.getComponent<Renderable>(object);

		if (objectRenderable.loaded && !objectRenderable.destroyed) {
			objectRenderable.model->bindBuffers(&renderingCommandBuffers[frameInFlightIndex]);

			// Opaque
			if (objectRenderable.model->gotOpaquePrimitives) {
				depthPrepass.opaqueGraphicsPipeline.bind(&renderingCommandBuffers[frameInFlightIndex]);
				objectRenderable.depthPrepassDescriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], &depthPrepass.opaqueGraphicsPipeline, 0);
				objectRenderable.drawOpaque(&renderingCommandBuffers[frameInFlightIndex], &depthPrepass.opaqueGraphicsPipeline, false, frameInFlightIndex, true);
			}

			// Mask
			if (objectRenderable.model->gotMaskPrimitives) {
				depthPrepass.maskGraphicsPipeline.bind(&renderingCommandBuffers[frameInFlightIndex]);
				objectRenderable.depthPrepassMaskDescriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], &depthPrepass.maskGraphicsPipeline, 0);
				objectRenderable.drawMask(&renderingCommandBuffers[frameInFlightIndex], &depthPrepass.maskGraphicsPipeline, true, frameInFlightIndex, true);
			}
		}
	}

	depthPrepass.renderPass.end(&renderingCommandBuffers[frameInFlightIndex]);

	// Shadow
	int shadowPushConstants[2];
	for (Entity light : *lights) {
		auto const& lightLight = ecs.getComponent<Light>(light);

		shadowPushConstants[0] = lightLight.shadowMapIndex;
		shadowPushConstants[1] = (lightLight.component.type == LightType::DIRECTIONAL) ? 0 : ((lightLight.component.type == LightType::POINT) ? 1 : 2);

		if ((lightLight.component.type == LightType::DIRECTIONAL || lightLight.component.type == LightType::SPOT) && lightLight.shadowMapIndex != 0) {
			Framebuffer* shadowFramebuffer = (lightLight.component.type == LightType::DIRECTIONAL) ? &shadow.directionalFramebuffers[lightLight.shadowMapIndex - 1] : &shadow.spotFramebuffers[lightLight.shadowMapIndex - 1];
			shadow.renderPass.begin(&renderingCommandBuffers[frameInFlightIndex], shadowFramebuffer->framebuffer, { SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT });

			for (Entity object : entities) {
				auto& objectRenderable = ecs.getComponent<Renderable>(object);

				if (objectRenderable.loaded && !objectRenderable.destroyed) {
					objectRenderable.model->bindBuffers(&renderingCommandBuffers[frameInFlightIndex]);

					// Opaque
					if (objectRenderable.model->gotOpaquePrimitives) {
						shadow.opaqueGraphicsPipeline.bind(&renderingCommandBuffers[frameInFlightIndex]);
						objectRenderable.shadowDescriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], &shadow.opaqueGraphicsPipeline, 0);
						shadow.opaqueGraphicsPipeline.pushConstant(&renderingCommandBuffers[frameInFlightIndex], VK_SHADER_STAGE_VERTEX_BIT, 0, 2 * sizeof(int), &shadowPushConstants);
						objectRenderable.drawOpaque(&renderingCommandBuffers[frameInFlightIndex], &shadow.opaqueGraphicsPipeline, false, frameInFlightIndex, false);
					}

					// Mask
					if (objectRenderable.model->gotMaskPrimitives) {
						shadow.maskGraphicsPipeline.bind(&renderingCommandBuffers[frameInFlightIndex]);
						objectRenderable.shadowMaskDescriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], &shadow.maskGraphicsPipeline, 0);
						shadow.maskGraphicsPipeline.pushConstant(&renderingCommandBuffers[frameInFlightIndex], VK_SHADER_STAGE_VERTEX_BIT, 0, 2 * sizeof(int), &shadowPushConstants);
						objectRenderable.drawMask(&renderingCommandBuffers[frameInFlightIndex], &shadow.maskGraphicsPipeline, true, frameInFlightIndex, false);
					}
				}
			}

			shadow.renderPass.end(&renderingCommandBuffers[frameInFlightIndex]);
		}
	}

	// Opaque scene
	opaqueSceneRenderPass->begin(&renderingCommandBuffers[frameInFlightIndex], opaqueSceneFramebuffer.framebuffer, window.extent);
	for (Entity object : entities) {
		auto& objectRenderable = ecs.getComponent<Renderable>(object);

		if (objectRenderable.loaded && !objectRenderable.destroyed) {
			objectRenderable.model->bindBuffers(&renderingCommandBuffers[frameInFlightIndex]);

			// Opaque
			if (objectRenderable.model->gotOpaquePrimitives) {
				objectRenderable.opaqueGraphicsPipeline->bind(&renderingCommandBuffers[frameInFlightIndex]);
				if (objectRenderable.opaqueGraphicsPipeline->sets.size() != 0) {
					objectRenderable.descriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], objectRenderable.opaqueGraphicsPipeline, 0);
				}
				objectRenderable.drawOpaque(&renderingCommandBuffers[frameInFlightIndex], objectRenderable.opaqueGraphicsPipeline, true, frameInFlightIndex, true);
			}

			// Mask
			if (objectRenderable.model->gotMaskPrimitives) {
				objectRenderable.maskGraphicsPipeline->bind(&renderingCommandBuffers[frameInFlightIndex]);
				if (objectRenderable.maskGraphicsPipeline->sets.size() != 0) {
					objectRenderable.descriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], objectRenderable.maskGraphicsPipeline, 0);
				}
				objectRenderable.drawMask(&renderingCommandBuffers[frameInFlightIndex], objectRenderable.maskGraphicsPipeline, true, frameInFlightIndex, true);
			}
		}
	}
	if (scene.skyboxType == SkyboxType::ENVMAP) {
		envmap.skyboxGraphicsPipeline.bind(&renderingCommandBuffers[frameInFlightIndex]);
		envmap.skyboxDescriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], &envmap.skyboxGraphicsPipeline, 0);

		envmap.draw(&renderingCommandBuffers[frameInFlightIndex]);
	}

	opaqueSceneRenderPass->end(&renderingCommandBuffers[frameInFlightIndex]);

	if (scene.skyboxType == SkyboxType::ATMOSPHERE) {
		// Atmosphere
		atmosphere.draw(&renderingCommandBuffers[frameInFlightIndex], frameInFlightIndex);
	}

	// Blend scene
	blendSceneRenderPass->begin(&renderingCommandBuffers[frameInFlightIndex], blendSceneFramebuffer.framebuffer, window.extent);
	for (Entity object : entities) {
		auto& objectRenderable = ecs.getComponent<Renderable>(object);
		
		if (objectRenderable.loaded && !objectRenderable.destroyed) {
			objectRenderable.model->bindBuffers(&renderingCommandBuffers[frameInFlightIndex]);

			if (objectRenderable.model->gotBlendPrimitives) {
				objectRenderable.blendGraphicsPipeline->bind(&renderingCommandBuffers[frameInFlightIndex]);
				if (objectRenderable.blendGraphicsPipeline->sets.size() != 0) {
					objectRenderable.descriptorSets.at(frameInFlightIndex).bind(&renderingCommandBuffers[frameInFlightIndex], objectRenderable.blendGraphicsPipeline, 0);
				}
				objectRenderable.drawBlend(&renderingCommandBuffers[frameInFlightIndex], objectRenderable.blendGraphicsPipeline, true, frameInFlightIndex, true);
			}
		}
	}

	blendSceneRenderPass->end(&renderingCommandBuffers[frameInFlightIndex]);

	// Alpha compositing
	GraphicsPipeline* alphaCompositingGraphicsPipeline = &graphicsPipelines.at("alphaCompositing");

	alphaCompositingRenderPass->begin(&renderingCommandBuffers[frameInFlightIndex], alphaCompositingFramebuffer.framebuffer, window.extent);
	alphaCompositingGraphicsPipeline->bind(&renderingCommandBuffers[frameInFlightIndex]);
	alphaCompositingDescriptorSet.bind(&renderingCommandBuffers[frameInFlightIndex], alphaCompositingGraphicsPipeline, 0);
	
	vkCmdDraw(renderingCommandBuffers[frameInFlightIndex].commandBuffer, 3, 1, 0, 0);

	alphaCompositingRenderPass->end(&renderingCommandBuffers[frameInFlightIndex]);

	// Bloom
	if (enableBloom) {
		bloom.draw(&renderingCommandBuffers[frameInFlightIndex]);
	}

	// SSAO
	if (enableSSAO) {
		ssao.draw(&renderingCommandBuffers[frameInFlightIndex], frameInFlightIndex);
	}

	// Post-processing
	GraphicsPipeline* postGraphicsPipeline = &graphicsPipelines.at("post");

	VkFramebuffer postProcessFramebuffer = enableFXAA ? postFramebuffers[0].framebuffer : postFramebuffers[framebufferIndex].framebuffer;
	postRenderPass->begin(&renderingCommandBuffers[frameInFlightIndex], postProcessFramebuffer, window.extent);
	postGraphicsPipeline->bind(&renderingCommandBuffers[frameInFlightIndex]);
	postDescriptorSet.bind(&renderingCommandBuffers[frameInFlightIndex], postGraphicsPipeline, 0);
	VkBool32 parameters[2] = { enableSSAO, enableBloom };
	postGraphicsPipeline->pushConstant(&renderingCommandBuffers[frameInFlightIndex], VK_SHADER_STAGE_FRAGMENT_BIT, 0, 2 * sizeof(VkBool32), &parameters);

	vkCmdDraw(renderingCommandBuffers[frameInFlightIndex].commandBuffer, 3, 1, 0, 0);

	postRenderPass->end(&renderingCommandBuffers[frameInFlightIndex]);

	// FXAA
	if (enableFXAA) {
		fxaa.draw(&renderingCommandBuffers[frameInFlightIndex], framebufferIndex);
	}

	// UI
	ui.draw(&renderingCommandBuffers[frameInFlightIndex], framebufferIndex);

	renderingCommandBuffers[frameInFlightIndex].end();
}

void Renderer::createResources() {
	// Opaque scene
	{
		ImageTools::createImage(&sceneImage.image, 1, window.extent.width, window.extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &sceneImage.memoryInfo);
		ImageTools::createImageView(&sceneImage.imageView, sceneImage.image, 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
	
		std::vector<VkImageView> framebufferAttachments;
		framebufferAttachments.push_back(sceneImage.imageView);
		framebufferAttachments.push_back(defaultRevealageAttachment.imageView);
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

		std::vector<std::vector<VkImageView>> framebufferAttachments;
		uint32_t postProcessSize;
		if (enableFXAA) {
			postProcessSize = 1;
		}
		else {
			postProcessSize = swapchainSize;
		}
		framebufferAttachments.resize(postProcessSize);
		postFramebuffers.resize(postProcessSize);
		for (uint32_t i = 0; i < postProcessSize; i++) {
			if (enableFXAA) {
				framebufferAttachments[i].push_back(postProcessImage.imageView);
			}
			else {
				framebufferAttachments[i].push_back(swapchain.imageViews[i]);
			}
			postFramebuffers[i].init(&renderPasses.at("post"), framebufferAttachments[i], window.extent.width, window.extent.height, 1);
		}
	}
}

void Renderer::destroyResources() {
	sceneImage.destroy();
	blendAccumulationImage.destroy();
	blendRevealageImage.destroy();
	postProcessImage.destroy();
	opaqueSceneFramebuffer.destroy();
	blendSceneFramebuffer.destroy();
	alphaCompositingFramebuffer.destroy();
	for (Framebuffer& framebuffer : postFramebuffers) {
		framebuffer.destroy();
	}
	postFramebuffers.clear();
	postFramebuffers.shrink_to_fit();
}

void Renderer::createAdditionalDescriptorSets() {
	// Texture and Materials
	// Descriptor Pool
	VkDescriptorPoolSize texturePoolSize = {};
	texturePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	texturePoolSize.descriptorCount = 524288;

	VkDescriptorPoolSize materialPoolSize = {};
	materialPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	materialPoolSize.descriptorCount = 524288;

	VkDescriptorPoolCreateInfo materialPoolCreateInfo = {};
	materialPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	materialPoolCreateInfo.pNext = nullptr;
	materialPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	materialPoolCreateInfo.maxSets = framesInFlight;
	materialPoolCreateInfo.poolSizeCount = 2;
	std::array<VkDescriptorPoolSize, 2> descriptorPools = { texturePoolSize, materialPoolSize };
	materialPoolCreateInfo.pPoolSizes = descriptorPools.data();
	NEIGE_VK_CHECK(vkCreateDescriptorPool(logicalDevice.device, &materialPoolCreateInfo, nullptr, &materialsDescriptorPool.descriptorPool));

	materialsDescriptorPool.remainingSets = 0;

	// Descriptor Set Layout
	VkDescriptorSetLayoutBinding textureBinding = {};
	textureBinding.binding = 0;
	textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureBinding.descriptorCount = 524288;
	textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	textureBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding materialBinding = {};
	materialBinding.binding = 1;
	materialBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	materialBinding.descriptorCount = 524288;
	materialBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	materialBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBindingFlagsCreateInfo materialLayoutBindingFlagsCreateInfo = {};
	materialLayoutBindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	materialLayoutBindingFlagsCreateInfo.pNext = nullptr;
	materialLayoutBindingFlagsCreateInfo.bindingCount = 2;
	std::array<VkDescriptorBindingFlags, 2> flags = { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT };
	materialLayoutBindingFlagsCreateInfo.pBindingFlags = flags.data();

	VkDescriptorSetLayoutCreateInfo materialSetLayoutCreateInfo = {};
	materialSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	materialSetLayoutCreateInfo.pNext = &materialLayoutBindingFlagsCreateInfo;
	materialSetLayoutCreateInfo.flags = 0;
	materialSetLayoutCreateInfo.bindingCount = 2;
	std::array<VkDescriptorSetLayoutBinding, 2> descriptorSetLayoutBindings = { textureBinding, materialBinding };
	materialSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();
	NEIGE_VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice.device, &materialSetLayoutCreateInfo, nullptr, &materialsDescriptorSetLayout));

	// Allocation
	materialsDescriptorSets.resize(framesInFlight);
	materialDescriptorSetUpToDate.resize(framesInFlight);
	for (uint32_t i = 0; i < framesInFlight; i++) {
		VkDescriptorSetAllocateInfo materialAllocateInfo = {};
		materialAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		materialAllocateInfo.pNext = nullptr;
		materialAllocateInfo.descriptorPool = materialsDescriptorPool.descriptorPool;
		materialAllocateInfo.descriptorSetCount = 1;
		materialAllocateInfo.pSetLayouts = &materialsDescriptorSetLayout;
		NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &materialAllocateInfo, &materialsDescriptorSets[i].descriptorSet));

		materialsDescriptorSets[i].descriptorPool = &materialsDescriptorPool;
		materialDescriptorSetUpToDate[i] = false;
	}

	// Per draw information
	// Descriptor Pool
	VkDescriptorPoolSize perDrawPoolSize = {};
	perDrawPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	perDrawPoolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo perDrawPoolCreateInfo = {};
	perDrawPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	perDrawPoolCreateInfo.pNext = nullptr;
	perDrawPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	perDrawPoolCreateInfo.maxSets = 131072;
	perDrawPoolCreateInfo.poolSizeCount = 1;
	perDrawPoolCreateInfo.pPoolSizes = &perDrawPoolSize;
	NEIGE_VK_CHECK(vkCreateDescriptorPool(logicalDevice.device, &perDrawPoolCreateInfo, nullptr, &perDrawDescriptorPool.descriptorPool));

	perDrawDescriptorPool.remainingSets = 131072;

	// Descriptor Set Layout
	VkDescriptorSetLayoutBinding perDrawBinding = {};
	perDrawBinding.binding = 0;
	perDrawBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	perDrawBinding.descriptorCount = 1;
	perDrawBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	perDrawBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo perDrawSetLayoutCreateInfo = {};
	perDrawSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	perDrawSetLayoutCreateInfo.pNext = nullptr;
	perDrawSetLayoutCreateInfo.flags = 0;
	perDrawSetLayoutCreateInfo.bindingCount = 1;
	perDrawSetLayoutCreateInfo.pBindings = &perDrawBinding;
	NEIGE_VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice.device, &perDrawSetLayoutCreateInfo, nullptr, &perDrawDescriptorSetLayout));
}

void Renderer::updateMaterialDescriptorSet(uint32_t frameInFlightIndex) {
	std::vector<VkDescriptorImageInfo> textureInfos;
	for (Texture& texture : textures) {
		VkDescriptorImageInfo textureInfo = {};
		textureInfo.sampler = trilinearRepeatBlackSampler;
		textureInfo.imageView = texture.image.imageView;
		textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		textureInfos.push_back(textureInfo);
	}

	std::vector<VkDescriptorBufferInfo> materialsInfos;
	for (Material& material : materials) {
		VkDescriptorBufferInfo materialInfo = {};
		materialInfo.buffer = material.buffer.buffer;
		materialInfo.offset = 0;
		materialInfo.range = 5 * sizeof(int);

		materialsInfos.push_back(materialInfo);
	}

	std::vector<VkWriteDescriptorSet> writesDescriptorSet;

	VkWriteDescriptorSet textureWriteDescriptorSet = {};
	textureWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	textureWriteDescriptorSet.pNext = nullptr;
	textureWriteDescriptorSet.dstSet = materialsDescriptorSets[frameInFlightIndex].descriptorSet;
	textureWriteDescriptorSet.dstBinding = 0;
	textureWriteDescriptorSet.dstArrayElement = 0;
	textureWriteDescriptorSet.descriptorCount = static_cast<uint32_t>(textures.size());
	textureWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureWriteDescriptorSet.pImageInfo = textureInfos.data();
	textureWriteDescriptorSet.pBufferInfo = nullptr;
	textureWriteDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(textureWriteDescriptorSet);

	VkWriteDescriptorSet materialWriteDescriptorSet = {};
	materialWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	materialWriteDescriptorSet.pNext = nullptr;
	materialWriteDescriptorSet.dstSet = materialsDescriptorSets[frameInFlightIndex].descriptorSet;
	materialWriteDescriptorSet.dstBinding = 1;
	materialWriteDescriptorSet.dstArrayElement = 0;
	materialWriteDescriptorSet.descriptorCount = static_cast<uint32_t>(materials.size());
	materialWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	materialWriteDescriptorSet.pImageInfo = nullptr;
	materialWriteDescriptorSet.pBufferInfo = materialsInfos.data();
	materialWriteDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(materialWriteDescriptorSet);

	materialsDescriptorSets[frameInFlightIndex].update(writesDescriptorSet);
}

void Renderer::createAlphaCompositingDescriptorSet() {
	alphaCompositingDescriptorSet.init(&graphicsPipelines.at("alphaCompositing"), 0);

	VkDescriptorImageInfo accumulationInfo = {};
	accumulationInfo.sampler = trilinearOffscreenSampler;
	accumulationInfo.imageView = blendAccumulationImage.imageView;
	accumulationInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkDescriptorImageInfo revealageInfo = {};
	revealageInfo.sampler = trilinearOffscreenSampler;
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
	VkDescriptorImageInfo bloomInfo = {};
	VkDescriptorImageInfo ssaoInfo = {};

	sceneInfo.sampler = trilinearOffscreenSampler;
	sceneInfo.imageView = sceneImage.imageView;
	sceneInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	bloomInfo.sampler = trilinearLodOffscreenSampler;
	bloomInfo.imageView = enableBloom ? bloom.bloomImage.imageView : defaultPostProcessEffectImage.imageView;
	bloomInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ssaoInfo.sampler = nearestOffscreenSampler;
	ssaoInfo.imageView = enableSSAO ? ssao.ssaoBlurredImage.imageView : defaultPostProcessEffectImage.imageView;
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

	VkWriteDescriptorSet ssaoWriteDescriptorSet = {};
	ssaoWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	ssaoWriteDescriptorSet.pNext = nullptr;
	ssaoWriteDescriptorSet.dstSet = postDescriptorSet.descriptorSet;
	ssaoWriteDescriptorSet.dstBinding = 1;
	ssaoWriteDescriptorSet.dstArrayElement = 0;
	ssaoWriteDescriptorSet.descriptorCount = 1;
	ssaoWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	ssaoWriteDescriptorSet.pImageInfo = &ssaoInfo;
	ssaoWriteDescriptorSet.pBufferInfo = nullptr;
	ssaoWriteDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(ssaoWriteDescriptorSet);

	VkWriteDescriptorSet bloomWriteDescriptorSet = {};
	bloomWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	bloomWriteDescriptorSet.pNext = nullptr;
	bloomWriteDescriptorSet.dstSet = postDescriptorSet.descriptorSet;
	bloomWriteDescriptorSet.dstBinding = 2;
	bloomWriteDescriptorSet.dstArrayElement = 0;
	bloomWriteDescriptorSet.descriptorCount = 1;
	bloomWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bloomWriteDescriptorSet.pImageInfo = &bloomInfo;
	bloomWriteDescriptorSet.pBufferInfo = nullptr;
	bloomWriteDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(bloomWriteDescriptorSet);

	postDescriptorSet.update(writesDescriptorSet);
}

void Renderer::reloadOnResize() {
	while (window.extent.width == 0 || window.extent.height == 0) {
		window.waitEvents();
	}
	logicalDevice.wait();

	destroyResources();

	// Swapchain
	swapchain.destroyResources();
	swapchain.init(&swapchainSize);

	fullscreenViewport.viewport.width = static_cast<float>(window.extent.width);
	fullscreenViewport.viewport.height = static_cast<float>(window.extent.height);
	fullscreenViewport.scissor.extent.width = window.extent.width;
	fullscreenViewport.scissor.extent.height = window.extent.height;

	// Depth prepass
	depthPrepass.destroyResources();
	depthPrepass.createResources(fullscreenViewport);

	// SSAO
	if (enableSSAO) {
		ssao.destroyResources();
		ssao.createResources(fullscreenViewport);
	}

	// Image and framebuffers
	createResources();

	// Atmosphere
	if (scene.skyboxType == SkyboxType::ATMOSPHERE) {
		atmosphere.destroyResources();
		atmosphere.createResources(fullscreenViewport);
	}

	// Bloom
	if (enableBloom) {
		bloom.destroyResources();
		bloom.createResources(fullscreenViewport);
	}

	// UI
	ui.destroyResources();
	ui.createResources(fullscreenViewport);

	// FXAA
	if (enableFXAA) {
		fxaa.destroyResources();
		fxaa.createResources(fullscreenViewport);
	}

	createAlphaCompositingDescriptorSet();

	createPostProcessDescriptorSet();

	for (Entity camera : cameraSystem->entities) {
		auto& cameraCamera = ecs.getComponent<Camera>(camera);
		cameraCamera.projection = Camera::createPerspectiveProjection(cameraCamera.component.FOV, window.extent.width / static_cast<float>(window.extent.height), cameraCamera.component.nearPlane, cameraCamera.component.farPlane, true);
	}
}