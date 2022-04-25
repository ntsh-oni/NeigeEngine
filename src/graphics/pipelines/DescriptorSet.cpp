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

void DescriptorSet::update() {
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

void DescriptorSet::addWriteCombinedImageSampler(uint32_t dstBinding, uint32_t descriptorCount, VkDescriptorImageInfo* descriptorImageInfos) {
	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.pNext = nullptr;
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.dstBinding = dstBinding;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorCount = descriptorCount;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.pImageInfo = descriptorImageInfos;
	writeDescriptorSet.pBufferInfo = nullptr;
	writeDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(writeDescriptorSet);
}

void DescriptorSet::addWriteUniformBuffer(uint32_t dstBinding, uint32_t descriptorCount, VkDescriptorBufferInfo* descriptorBufferInfos) {
	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.pNext = nullptr;
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.dstBinding = dstBinding;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorCount = descriptorCount;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.pImageInfo = nullptr;
	writeDescriptorSet.pBufferInfo = descriptorBufferInfos;
	writeDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(writeDescriptorSet);
}

void DescriptorSet::addWriteStorageBuffer(uint32_t dstBinding, uint32_t descriptorCount, VkDescriptorBufferInfo* descriptorBufferInfos) {
	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.pNext = nullptr;
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.dstBinding = dstBinding;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorCount = descriptorCount;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeDescriptorSet.pImageInfo = nullptr;
	writeDescriptorSet.pBufferInfo = descriptorBufferInfos;
	writeDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(writeDescriptorSet);
}

void DescriptorSet::addWriteStorageImage(uint32_t dstBinding, uint32_t descriptorCount, VkDescriptorImageInfo* descriptorImageInfos) {
	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.pNext = nullptr;
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.dstBinding = dstBinding;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorCount = descriptorCount;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	writeDescriptorSet.pImageInfo = descriptorImageInfos;
	writeDescriptorSet.pBufferInfo = nullptr;
	writeDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(writeDescriptorSet);
}
