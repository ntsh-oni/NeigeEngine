#pragma once
#include "vulkan/vulkan.h"
#include "../../utils/NeigeDefines.h"
#include "../structs/ShaderStructs.h"
#include "../commands/CommandBuffer.h"
#include "DescriptorPool.h"
#include "Shader.h"
#include <vector>

struct ComputePipeline {
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	std::vector<DescriptorPool> descriptorPools;
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	std::string computeShaderPath;
	std::vector<Set> sets;
	std::vector<VkPushConstantRange> pushConstantRanges;

	// Parameters
	std::vector<int> externalSets;
	std::vector<VkDescriptorSetLayout> externalDescriptorSetLayouts;

	void init();
	void destroy();
	DescriptorPool* getDescriptorPool(uint32_t setsToAllocate);
	void bind(CommandBuffer* commandBuffer);
	void pushConstant(CommandBuffer* commandBuffer, VkShaderStageFlags stages, uint32_t offset, uint32_t size, const void* data);
	void destroyPipeline();
	int externalSet(size_t index);
};
