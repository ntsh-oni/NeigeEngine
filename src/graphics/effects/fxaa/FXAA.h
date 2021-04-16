#pragma once
#include "vulkan/vulkan.hpp"
#include "../../../utils/structs/ShaderStructs.h"
#include "../../resources/Image.h"
#include "../../renderpasses/Framebuffer.h"
#include "../../renderpasses/RenderPass.h"
#include "../../pipelines/DescriptorSet.h"
#include "../../pipelines/GraphicsPipeline.h"
#include "../../pipelines/Viewport.h"

struct FXAA {
	Viewport viewport;

	GraphicsPipeline graphicsPipeline;
	DescriptorSet descriptorSet;
	RenderPass renderPass;
	std::vector<Framebuffer> framebuffers;

	void init(Viewport fullscreenViewport);
	void destroy();
	void createResources(Viewport fullscreenViewport);
	void destroyResources();
	void draw(CommandBuffer* commandBuffer, uint32_t framebufferIndex);
};