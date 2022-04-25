#pragma once
#include "vulkan/vulkan.h"
#include "ComputePipeline.h"
#include "GraphicsPipeline.h"
#include "DescriptorPool.h"
#include "../commands/CommandBuffer.h"
#include <vector>

struct DescriptorSet {
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	DescriptorPool* descriptorPool;
	std::vector<VkWriteDescriptorSet> writesDescriptorSet;

	void init(GraphicsPipeline* associatedGraphicsPipeline, uint32_t set);
	void init(ComputePipeline* associatedComputePipeline, uint32_t set);
	void update();
	void destroy();
	void bind(CommandBuffer* commandBuffer, GraphicsPipeline* graphicsPipeline, uint32_t set);
	void bind(CommandBuffer* commandBuffer, ComputePipeline* computePipeline, uint32_t set);
	void addWriteCombinedImageSampler(uint32_t dstBinding, uint32_t descriptorCount, VkDescriptorImageInfo* descriptorImageInfos);
	void addWriteUniformBuffer(uint32_t dstBinding, uint32_t descriptorCount, VkDescriptorBufferInfo* descriptorBufferInfos);
	void addWriteStorageBuffer(uint32_t dstBinding, uint32_t descriptorCount, VkDescriptorBufferInfo* descriptorBufferInfos);
	void addWriteStorageImage(uint32_t dstBinding, uint32_t descriptorCount, VkDescriptorImageInfo* descriptorImageInfos);
};