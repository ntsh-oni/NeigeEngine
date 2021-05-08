#pragma once
#include "vulkan/vulkan.h"
#include "ComputePipeline.h"
#include "GraphicsPipeline.h"
#include "../commands/CommandBuffer.h"

struct DescriptorSet {
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	DescriptorPool* descriptorPool;

	void init(GraphicsPipeline* associatedGraphicsPipeline, uint32_t set);
	void init(ComputePipeline* associatedComputePipeline, uint32_t set);
	void update(const std::vector<VkWriteDescriptorSet>& writeDescriptorSets);
	void destroy();
	void bind(CommandBuffer* commandBuffer, GraphicsPipeline* graphicsPipeline, uint32_t set);
	void bind(CommandBuffer* commandBuffer, ComputePipeline* computePipeline, uint32_t set);
};