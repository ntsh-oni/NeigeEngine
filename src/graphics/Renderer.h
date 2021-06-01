#pragma once
#include "vulkan/vulkan.h"
#include "../utils/NeigeDefines.h"
#include "structs/RendererStructs.h"
#include "structs/ShaderStructs.h"
#include "structs/UIStructs.h"
#include "../utils/NeigeVKTranslate.h"
#include "../utils/resources/ImageTools.h"
#include "../utils/resources/ModelLoader.h"
#include "devices/PhysicalDevicePicker.h"
#include "commands/CommandBuffer.h"
#include "commands/CommandPool.h"
#include "pipelines/GraphicsPipeline.h"
#include "pipelines/DescriptorSet.h"
#include "pipelines/Shader.h"
#include "pipelines/Viewport.h"
#include "resources/Image.h"
#include "renderpasses/Framebuffer.h"
#include "renderpasses/RenderPass.h"
#include "sync/Fence.h"
#include "sync/Semaphore.h"
#include "../ecs/ECS.h"
#include "../ecs/systems/Lighting.h"
#include "../ecs/systems/CameraSystem.h"
#include "../Scene.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <map>

struct Renderer : public System {
	// Internal systems
	std::shared_ptr<Lighting> lighting;
	std::shared_ptr<CameraSystem> cameraSystem;

	// Sync
	std::vector<Fence> fences;
	std::vector<Semaphore> IAsemaphores;
	std::vector<Semaphore> RFsemaphores;

	// Pipelines
	Viewport fullscreenViewport;
	std::unordered_map<std::string, GraphicsPipeline> graphicsPipelines;

	// Render Passes
	std::unordered_map<std::string, RenderPass> renderPasses;

	Framebuffer opaqueSceneFramebuffer;
	Framebuffer blendSceneFramebuffer;
	Framebuffer alphaCompositingFramebuffer;
	std::vector<Framebuffer> postFramebuffers;

	DescriptorSet alphaCompositingDescriptorSet;
	DescriptorSet postDescriptorSet;

	Image defaultRevealageAttachment;
	Image defaultPostProcessEffectImage;

	// Command buffers
	std::vector<CommandPool> renderingCommandPools;
	std::vector<CommandBuffer> renderingCommandBuffers;

	uint32_t currentFrame = 0;

	// Scene
	SkyboxType skyboxType;
	std::string envmapPath;

	// Settings
	bool enableBloom;
	int bloomDownscale;
	float bloomThreshold;
	int bloomBlurSize;
	bool enableSSAO;
	int ssaoDownscale;
	bool enableFXAA;

	void init(const std::string& applicationName);
	void update();
	void destroy();
	void loadObject(Entity object);
	void updateData(uint32_t frameInFlightIndex);
	void recordRenderingCommands(uint32_t frameInFlightIndex, uint32_t framebufferIndex);
	void createResources();
	void destroyResources();
	void createAdditionalDescriptorSets();
	void updateMaterialDescriptorSet(uint32_t frameInFlightIndex);
	void createAlphaCompositingDescriptorSet();
	void createPostProcessDescriptorSet();
	void reloadOnResize();
};