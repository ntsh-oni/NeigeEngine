#include "ComputePipeline.h"
#include "../resources/RendererResources.h"
#include "../resources/ShaderResources.h"

void ComputePipeline::init() {
	sets.clear();
	sets.shrink_to_fit();
	pushConstantRanges.clear();
	pushConstantRanges.shrink_to_fit();

	std::vector<VkPipelineShaderStageCreateInfo> pipelineStages;
	std::set<VkDescriptorType> uniqueDescriptorTypes;

	if (computeShaderPath != "") {
		Shader shader;
		std::unordered_map<std::string, Shader>::const_iterator mapSearch = shaders.find(computeShaderPath);
		if (mapSearch == shaders.end()) {
			shader.init(computeShaderPath);
			shaders.emplace(computeShaderPath, shader);
		}
		else {
			shader = shaders[computeShaderPath];
		}
		NEIGE_ASSERT(shader.type == ShaderType::COMPUTE, "Compute shader in pipeline is not a vertex shader.");

		VkPipelineShaderStageCreateInfo computeShaderCreateInfo = {};
		computeShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderCreateInfo.pNext = nullptr;
		computeShaderCreateInfo.flags = 0;
		computeShaderCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderCreateInfo.module = shader.module;
		computeShaderCreateInfo.pName = "main";
		computeShaderCreateInfo.pSpecializationInfo = nullptr;
		pipelineStages.push_back(computeShaderCreateInfo);

		for (size_t i = 0; i < shader.sets.size(); i++) {
			bool found = false;
			for (size_t j = 0; j < sets.size(); j++) {
				if (sets[j].set == shader.sets[i].set) {
					sets[j].bindings.insert(sets[j].bindings.end(), shader.sets[i].bindings.begin(), shader.sets[i].bindings.end());
					found = true;
				}
			}
			if (!found) {
				sets.push_back(shader.sets[i]);
			}
		}
		pushConstantRanges.insert(pushConstantRanges.end(), shader.pushConstantRanges.begin(), shader.pushConstantRanges.end());
		uniqueDescriptorTypes.insert(shader.uniqueDescriptorTypes.begin(), shader.uniqueDescriptorTypes.end());
	}

	NEIGE_ASSERT(pipelineStages.size() != 0, "Graphics pipeline got no stage (no shader given).");

	// Sort sets
	std::sort(sets.begin(), sets.end(), [](Set a, Set b) { return a.set < b.set; });
	for (size_t i = 0; i < sets.size(); i++) {
		// Sort bindings
		std::sort(sets[i].bindings.begin(), sets[i].bindings.end(), [](Binding a, Binding b) { return a.binding.binding < b.binding.binding; });
	}

	std::vector<std::vector<VkDescriptorSetLayoutBinding>> setBindings;
	setBindings.resize(sets.size());
	for (size_t i = 0; i < sets.size(); i++) {
		for (size_t j = 0; j < sets[i].bindings.size(); j++) {
			setBindings[i].push_back(sets[i].bindings[j].binding);
		}
	}

	// Descriptor set layouts
	descriptorSetLayouts.resize(sets.size());
	for (size_t i = 0; i < sets.size(); i++) {
		if (externalSet(i) == -1 && descriptorSetLayouts[i] == VK_NULL_HANDLE) {
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
			descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutCreateInfo.pNext = nullptr;
			descriptorSetLayoutCreateInfo.flags = 0;
			descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(setBindings[i].size());
			descriptorSetLayoutCreateInfo.pBindings = setBindings[i].data();
			NEIGE_VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice.device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayouts[i]));
		}
	}
	// External sets
	for (size_t i = 0; i < externalSets.size(); i++) {
		descriptorSetLayouts[externalSets[i]] = externalDescriptorSetLayouts[i];
	}

	// Descriptor pool
	if (descriptorPools.empty()) {
		descriptorPools.resize(1);
		descriptorPools[0].descriptorPool = VK_NULL_HANDLE;
		descriptorPools[0].remainingSets = 0;
	}
	for (size_t i = 0; i < descriptorPools.size(); i++) {
		if (descriptorPools[i].descriptorPool == VK_NULL_HANDLE) {
			for (std::set<VkDescriptorType>::iterator it = uniqueDescriptorTypes.begin(); it != uniqueDescriptorTypes.end(); it++) {
				VkDescriptorPoolSize descriptorPoolSize = {};
				descriptorPoolSize.type = *it;
				descriptorPoolSize.descriptorCount = 2048;
				descriptorPoolSizes.push_back(descriptorPoolSize);
			}

			if (descriptorPoolSizes.size() != 0) {
				VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
				descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				descriptorPoolCreateInfo.pNext = nullptr;
				descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
				descriptorPoolCreateInfo.maxSets = 8192;
				descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
				descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
				NEIGE_VK_CHECK(vkCreateDescriptorPool(logicalDevice.device, &descriptorPoolCreateInfo, nullptr, &descriptorPools[i].descriptorPool));

				descriptorPools[i].remainingSets = 8192;
			}
		}
	}

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
	pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
	NEIGE_VK_CHECK(vkCreatePipelineLayout(logicalDevice.device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	// Pipeline
	VkComputePipelineCreateInfo computePipelineCreateInfo = {};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.flags = 0;
	computePipelineCreateInfo.stage = pipelineStages[0];
	computePipelineCreateInfo.layout = pipelineLayout;
	computePipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	computePipelineCreateInfo.basePipelineIndex = -1;
	NEIGE_VK_CHECK(vkCreateComputePipelines(logicalDevice.device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &pipeline));
}

void ComputePipeline::destroy() {
	for (size_t i = 0; i < descriptorPools.size(); i++) {
		if (descriptorPools[i].descriptorPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(logicalDevice.device, descriptorPools[i].descriptorPool, nullptr);
			descriptorPools[i].descriptorPool = VK_NULL_HANDLE;
			descriptorPools[i].remainingSets = 0;
		}
	}
	for (size_t i = 0; i < descriptorSetLayouts.size(); i++) {
		if (externalSet(i) == -1 && descriptorSetLayouts[i] != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(logicalDevice.device, descriptorSetLayouts[i], nullptr);
			descriptorSetLayouts[i] = VK_NULL_HANDLE;
		}
	}
	destroyPipeline();
}

DescriptorPool* ComputePipeline::getDescriptorPool(uint32_t setsToAllocate) {
	for (size_t i = 0; i < descriptorPools.size(); i++) {
		if (descriptorPools[i].descriptorPool != VK_NULL_HANDLE && descriptorPools[i].remainingSets >= setsToAllocate) {
			descriptorPools[i].remainingSets -= setsToAllocate;
			return &descriptorPools[i];
		}
	}

	// Create new descriptor pool
	DescriptorPool descriptorPool;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = nullptr;
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.maxSets = 8192;
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
	NEIGE_VK_CHECK(vkCreateDescriptorPool(logicalDevice.device, &descriptorPoolCreateInfo, nullptr, &descriptorPool.descriptorPool));

	descriptorPool.remainingSets = 8192 - setsToAllocate;

	descriptorPools.push_back(descriptorPool);

	return &descriptorPools[descriptorPools.size() - 1];
}

void ComputePipeline::bind(CommandBuffer* commandBuffer) {
	vkCmdBindPipeline(commandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
}

void ComputePipeline::pushConstant(CommandBuffer* commandBuffer, VkShaderStageFlags stages, uint32_t offset, uint32_t size, const void* data) {
	vkCmdPushConstants(commandBuffer->commandBuffer, pipelineLayout, stages, offset, size, data);
}

void ComputePipeline::destroyPipeline() {
	if (pipelineLayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(logicalDevice.device, pipelineLayout, nullptr);
		pipelineLayout = VK_NULL_HANDLE;
	}
	if (pipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(logicalDevice.device, pipeline, nullptr);
		pipeline = VK_NULL_HANDLE;
	}
}

int ComputePipeline::externalSet(size_t index) {
	for (size_t i = 0; i < externalSets.size(); i++) {
		if (externalSets[i] == index) {
			return static_cast<int>(i);
		}
	}

	return -1;
}
