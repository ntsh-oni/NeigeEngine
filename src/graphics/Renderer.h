#pragma once
#include "vulkan/vulkan.hpp"
#include "../utils/NeigeDefines.h"
#include "../utils/structs/RendererStructs.h"
#include "../utils/structs/ShaderStructs.h"
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
#include "effects/depthprepass/DepthPrepass.h"
#include "effects/envmap/Envmap.h"
#include "effects/shadowmapping/Shadow.h"
#include "effects/ssao/SSAO.h"
#include "../ecs/ECS.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>

struct Renderer : public System {
	// Envmap
	GraphicsPipeline skyboxGraphicsPipeline;
	std::vector<DescriptorSet> skyboxDescriptorSets;

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
	Framebuffer postFramebuffer;

	DescriptorSet alphaCompositingDescriptorSet;
	DescriptorSet postDescriptorSet;

	// Command buffers
	std::vector<CommandPool> renderingCommandPools;
	std::vector<CommandBuffer> renderingCommandBuffers;

	uint32_t currentFrame = 0;

	bool pressed = false;

	void init(const std::string applicationName);
	void update();
	void destroy();
	void loadObject(Entity object);
	void updateData(uint32_t frameInFlightIndex);
	void recordRenderingCommands(uint32_t frameInFlightIndex, uint32_t framebufferIndex);
	void createResources();
	void destroyResources();
	void createBindlessDescriptorSet();
	void updateBindlessDescriptorSet();
	void createAlphaCompositingDescriptorSet();
	void createPostProcessDescriptorSet();
	void reloadOnResize();
};