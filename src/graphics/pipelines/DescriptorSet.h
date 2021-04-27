#pragma once
#include "vulkan/vulkan.h"
#include "GraphicsPipeline.h"
#include "../commands/CommandBuffer.h"

struct DescriptorSet {
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	GraphicsPipeline* graphicsPipeline;

	void init(GraphicsPipeline* graphicsPipeline, uint32_t set);
	void init();
	void update(const std::vector<VkWriteDescriptorSet>& writeDescriptorSets);
	void destroy();
	void bind(CommandBuffer* commandBuffer, uint32_t set);
	void bind(CommandBuffer* commandBuffer, GraphicsPipeline* graphicsPipeline, uint32_t set);
};
