#include "FrustumCulling.h"
#include "../../../graphics/resources/RendererResources.h"
#include "../../../graphics/resources/ShaderResources.h"

void FrustumCulling::init() {
	computePipeline.computeShaderPath = "../shaders/frustumculling/frustumculling.comp";
	computePipeline.init();
}

void FrustumCulling::destroy() {
	computePipeline.destroy();
}

void FrustumCulling::drawCountsReset(CommandBuffer* commandBuffer, const std::vector<VkBuffer>& drawCountBuffers) {
	std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers;
	bufferMemoryBarriers.resize(drawCountBuffers.size());
	for (size_t i = 0; i < drawCountBuffers.size(); i++) {
		// Reset draw count to 0
		vkCmdFillBuffer(commandBuffer->commandBuffer, drawCountBuffers[i], 0, sizeof(uint32_t), 0);

		// Barrier
		bufferMemoryBarriers[i].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferMemoryBarriers[i].pNext = nullptr;
		bufferMemoryBarriers[i].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		bufferMemoryBarriers[i].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		bufferMemoryBarriers[i].srcQueueFamilyIndex = physicalDevice.queueFamilyIndices.computeFamily.value();
		bufferMemoryBarriers[i].dstQueueFamilyIndex = physicalDevice.queueFamilyIndices.computeFamily.value();
		bufferMemoryBarriers[i].buffer = drawCountBuffers[i];
		bufferMemoryBarriers[i].offset = 0;
		bufferMemoryBarriers[i].size = sizeof(uint32_t);
	}

	vkCmdPipelineBarrier(commandBuffer->commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, static_cast<uint32_t>(drawCountBuffers.size()), bufferMemoryBarriers.data(), 0, nullptr);
}

void FrustumCulling::computeIndirectFragmentBarrier(CommandBuffer* commandBuffer, const std::vector<VkBuffer>& indirectBuffers, const std::vector<VkBuffer>& drawCountBuffers, const std::vector<VkBuffer>& perDrawBuffers, const std::vector<uint32_t>& drawCounts) {
	size_t indirectBuffersSize = indirectBuffers.size();
	size_t drawCountBuffersSize = drawCountBuffers.size();
	size_t perDrawBuffersSize = perDrawBuffers.size();
	
	int sizeIndex = 0;

	// Indirect commands
	std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers;
	bufferMemoryBarriers.resize(indirectBuffersSize + drawCountBuffersSize + perDrawBuffersSize);
	for (size_t i = 0; i < indirectBuffersSize; i++) {
		bufferMemoryBarriers[i].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferMemoryBarriers[i].pNext = nullptr;
		bufferMemoryBarriers[i].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		bufferMemoryBarriers[i].dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
		bufferMemoryBarriers[i].srcQueueFamilyIndex = physicalDevice.queueFamilyIndices.computeFamily.value();
		bufferMemoryBarriers[i].dstQueueFamilyIndex = physicalDevice.queueFamilyIndices.graphicsFamily.value();
		bufferMemoryBarriers[i].buffer = indirectBuffers[i];
		bufferMemoryBarriers[i].offset = 0;
		bufferMemoryBarriers[i].size = drawCounts[sizeIndex] * sizeof(VkDrawIndexedIndirectCommand);
		sizeIndex++;
	}

	// Draw counts
	for (size_t i = indirectBuffersSize; i < indirectBuffersSize + drawCountBuffersSize; i++) {
		bufferMemoryBarriers[i].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferMemoryBarriers[i].pNext = nullptr;
		bufferMemoryBarriers[i].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		bufferMemoryBarriers[i].dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
		bufferMemoryBarriers[i].srcQueueFamilyIndex = physicalDevice.queueFamilyIndices.computeFamily.value();
		bufferMemoryBarriers[i].dstQueueFamilyIndex = physicalDevice.queueFamilyIndices.graphicsFamily.value();
		bufferMemoryBarriers[i].buffer = drawCountBuffers[i - indirectBuffersSize];
		bufferMemoryBarriers[i].offset = 0;
		bufferMemoryBarriers[i].size = sizeof(uint32_t);
	}

	sizeIndex = 0;

	// Per draw
	for (size_t i = indirectBuffersSize + drawCountBuffersSize; i < indirectBuffersSize + drawCountBuffersSize + perDrawBuffersSize; i++) {
		bufferMemoryBarriers[i].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferMemoryBarriers[i].pNext = nullptr;
		bufferMemoryBarriers[i].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		bufferMemoryBarriers[i].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		bufferMemoryBarriers[i].srcQueueFamilyIndex = physicalDevice.queueFamilyIndices.computeFamily.value();
		bufferMemoryBarriers[i].dstQueueFamilyIndex = physicalDevice.queueFamilyIndices.graphicsFamily.value();
		bufferMemoryBarriers[i].buffer = perDrawBuffers[i - (indirectBuffersSize + drawCountBuffersSize)];
		bufferMemoryBarriers[i].offset = 0;
		bufferMemoryBarriers[i].size = drawCounts[sizeIndex] * sizeof(PerDraw);
		sizeIndex++;
	}

	vkCmdPipelineBarrier(commandBuffer->commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, static_cast<uint32_t>(indirectBuffersSize + drawCountBuffersSize + perDrawBuffersSize), bufferMemoryBarriers.data(), 0, nullptr);
}