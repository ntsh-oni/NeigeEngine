#pragma once
#include "vulkan/vulkan.h"
#include "../../utils/NeigeDefines.h"
#include "../../utils/structs/ShaderStructs.h"
#include "../commands/CommandBuffer.h"
#include "../renderpasses/RenderPass.h"
#include "Shader.h"
#include "Viewport.h"
#include <vector>

enum struct Compare {
	LESS_OR_EQUAL,
	LESS,
	EQUAL
};

struct GraphicsPipeline {
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	std::vector<DescriptorPool> descriptorPools;
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	std::string vertexShaderPath;
	std::string fragmentShaderPath;
	std::string tesselationControlShaderPath;
	std::string tesselationEvaluationShaderPath;
	std::string geometryShaderPath;
	RenderPass* renderPass;
	Viewport* viewport;
	Topology topology = Topology::TRIANGLE_LIST;
	std::vector<Set> sets;
	std::vector<VkPushConstantRange> pushConstantRanges;

	// Parameters
	std::vector<Blending> blendings;
	bool multiSample = false;
	bool depthWrite = true;
	Compare depthCompare = Compare::LESS_OR_EQUAL;
	bool backfaceCulling = true;
	std::vector<int> externalSets;
	std::vector<VkDescriptorSetLayout> externalDescriptorSetLayouts;
	std::vector<int> specializationConstantValues;

	void init();
	void destroy();
	DescriptorPool* getDescriptorPool(uint32_t setsToAllocate);
	void bind(CommandBuffer* commandBuffer);
	void pushConstant(CommandBuffer* commandBuffer, VkShaderStageFlags stages, uint32_t offset, uint32_t size, const void* data);
	void destroyPipeline();
	VkPrimitiveTopology topologyToVkTopology();
	VkCompareOp compareToVkCompare();
	int externalSet(size_t index);
};
