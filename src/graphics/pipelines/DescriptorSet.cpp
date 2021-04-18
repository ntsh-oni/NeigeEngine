#include "DescriptorSet.h"
#include "../resources/RendererResources.h"
#include "../resources/ShaderResources.h"

void DescriptorSet::init(GraphicsPipeline* associatedGraphicsPipeline, uint32_t set) {
	graphicsPipeline = associatedGraphicsPipeline;

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = nullptr;
	descriptorSetAllocateInfo.descriptorPool = graphicsPipeline->descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &graphicsPipeline->descriptorSetLayouts[set];
	NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &descriptorSetAllocateInfo, &descriptorSet));
}

void DescriptorSet::init() {
	// Bindless
	// Descriptor Pool
	VkDescriptorPoolSize descriptorPoolSize = {};
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorPoolSize.descriptorCount = 524288;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = nullptr;
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = 1;
	descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;
	NEIGE_VK_CHECK(vkCreateDescriptorPool(logicalDevice.device, &descriptorPoolCreateInfo, nullptr, &texturesDescriptorPool));

	// Descriptor Set Layout
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
	descriptorSetLayoutBinding.binding = 0;
	descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorSetLayoutBinding.descriptorCount = 524288;
	descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingFlagsCreateInfo = {};
	descriptorSetLayoutBindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	descriptorSetLayoutBindingFlagsCreateInfo.pNext = nullptr;
	descriptorSetLayoutBindingFlagsCreateInfo.bindingCount = 1;
	VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
	descriptorSetLayoutBindingFlagsCreateInfo.pBindingFlags = &flags;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pNext = &descriptorSetLayoutBindingFlagsCreateInfo;
	descriptorSetLayoutCreateInfo.flags = 0;
	descriptorSetLayoutCreateInfo.bindingCount = 1;
	descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;
	NEIGE_VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice.device, &descriptorSetLayoutCreateInfo, nullptr, &texturesDescriptorSetLayout));

	// Allocation
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = nullptr;
	descriptorSetAllocateInfo.descriptorPool = texturesDescriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &texturesDescriptorSetLayout;
	NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &descriptorSetAllocateInfo, &descriptorSet));
}

void DescriptorSet::update(const std::vector<VkWriteDescriptorSet> writesDescriptorSet) {
	vkUpdateDescriptorSets(logicalDevice.device, static_cast<uint32_t>(writesDescriptorSet.size()), writesDescriptorSet.data(), 0, nullptr);
}

void DescriptorSet::destroy() {
	vkFreeDescriptorSets(logicalDevice.device, graphicsPipeline->descriptorPool, 1, &descriptorSet);
}

void DescriptorSet::bind(CommandBuffer* commandBuffer, uint32_t set) {
	vkCmdBindDescriptorSets(commandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->pipelineLayout, set, 1, &descriptorSet, 0, nullptr);
}

void DescriptorSet::bind(CommandBuffer* commandBuffer, GraphicsPipeline* graphicsPipeline, uint32_t set) {
	vkCmdBindDescriptorSets(commandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->pipelineLayout, set, 1, &descriptorSet, 0, nullptr);
}