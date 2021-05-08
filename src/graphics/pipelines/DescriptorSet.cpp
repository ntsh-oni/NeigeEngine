#include "DescriptorSet.h"
#include "../resources/RendererResources.h"
#include "../resources/ShaderResources.h"

void DescriptorSet::init(GraphicsPipeline* associatedGraphicsPipeline, uint32_t set) {
	descriptorPool = associatedGraphicsPipeline->getDescriptorPool(1);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = nullptr;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool->descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &associatedGraphicsPipeline->descriptorSetLayouts[set];
	NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &descriptorSetAllocateInfo, &descriptorSet));
}

void DescriptorSet::init(ComputePipeline* associatedComputePipeline, uint32_t set) {
	descriptorPool = associatedComputePipeline->getDescriptorPool(1);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = nullptr;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool->descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &associatedComputePipeline->descriptorSetLayouts[set];
	NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &descriptorSetAllocateInfo, &descriptorSet));
}

void DescriptorSet::update(const std::vector<VkWriteDescriptorSet>& writesDescriptorSet) {
	vkUpdateDescriptorSets(logicalDevice.device, static_cast<uint32_t>(writesDescriptorSet.size()), writesDescriptorSet.data(), 0, nullptr);
}

void DescriptorSet::destroy() {
	vkFreeDescriptorSets(logicalDevice.device, descriptorPool->descriptorPool, 1, &descriptorSet);
	descriptorPool->remainingSets += 1;
}

void DescriptorSet::bind(CommandBuffer* commandBuffer, GraphicsPipeline* graphicsPipeline, uint32_t set) {
	vkCmdBindDescriptorSets(commandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->pipelineLayout, set, 1, &descriptorSet, 0, nullptr);
}

void DescriptorSet::bind(CommandBuffer* commandBuffer, ComputePipeline* computePipeline, uint32_t set) {
	vkCmdBindDescriptorSets(commandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline->pipelineLayout, set, 1, &descriptorSet, 0, nullptr);
}