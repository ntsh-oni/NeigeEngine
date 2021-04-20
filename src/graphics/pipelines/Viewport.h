#pragma once
#include "vulkan/vulkan.h"
#include "../commands/CommandBuffer.h"

struct Viewport {
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineViewportStateCreateInfo viewportCreateInfo;

	void init(uint32_t width, uint32_t height);
	void setViewport(CommandBuffer* commandBuffer);
	void setScissor(CommandBuffer* commandBuffer);
};