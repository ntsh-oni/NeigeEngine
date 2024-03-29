#include "BufferTools.h"
#include "../../graphics/resources/RendererResources.h"

void BufferTools::createBuffer(VkBuffer& buffer,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags memoryProperties,
	MemoryInfo* memoryInfo) {
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	NEIGE_VK_CHECK(vkCreateBuffer(logicalDevice.device, &bufferCreateInfo, nullptr, &buffer));

	memoryAllocator.allocate(&buffer, memoryProperties, memoryInfo);
}

void BufferTools::createStagingBuffer(VkBuffer& buffer,
	VkDeviceSize size,
	MemoryInfo* memoryInfo) {
	createBuffer(buffer, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryInfo);
}

void BufferTools::createUniformBuffer(VkBuffer& buffer,
	VkDeviceSize size,
	MemoryInfo* memoryInfo) {
	createBuffer(buffer, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryInfo);
}

void BufferTools::createStorageBuffer(VkBuffer& buffer,
	VkDeviceSize size,
	MemoryInfo* memoryInfo) {
	createBuffer(buffer, size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryInfo);
}

void BufferTools::createIndirectBuffer(VkBuffer& buffer,
	VkDeviceSize size,
	MemoryInfo* memoryInfo) {
	createBuffer(buffer, size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryInfo);
}

void BufferTools::copyBuffer(VkBuffer srcBuffer,
	VkBuffer dstBuffer,
	VkDeviceSize size) {
	CommandPool commandPool;
	commandPool.init();
	CommandBuffer commandBuffer;
	commandBuffer.init(&commandPool);
	commandBuffer.begin();

	VkBufferCopy bufferCopy = {};
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = 0;
	bufferCopy.size = size;
	vkCmdCopyBuffer(commandBuffer.commandBuffer, srcBuffer, dstBuffer, 1, &bufferCopy);

	commandBuffer.endAndSubmit();
	commandPool.destroy();
}

void BufferTools::copyToImage(VkBuffer srcBuffer,
	VkImage dstImage,
	uint32_t width,
	uint32_t height,
	uint32_t arrayLayers,
	uint64_t stride,
	VkImageAspectFlags aspectFlags) {
	CommandPool commandPool;
	commandPool.init();
	CommandBuffer commandBuffer;
	commandBuffer.init(&commandPool);
	commandBuffer.begin();

	std::vector<VkBufferImageCopy> bufferImageCopies;
	for (uint32_t i = 0; i < arrayLayers; i++) {
		VkBufferImageCopy bufferImageCopy = {};
		bufferImageCopy.bufferOffset = static_cast<VkDeviceSize>(width) * static_cast<VkDeviceSize>(height) * i * 4 * stride;
		bufferImageCopy.bufferRowLength = 0;
		bufferImageCopy.bufferImageHeight = 0;
		bufferImageCopy.imageSubresource.aspectMask = aspectFlags;
		bufferImageCopy.imageSubresource.mipLevel = 0;
		bufferImageCopy.imageSubresource.baseArrayLayer = i;
		bufferImageCopy.imageSubresource.layerCount = arrayLayers;
		bufferImageCopy.imageOffset = { 0, 0, 0 };
		bufferImageCopy.imageExtent = { width, height, 1 };
		bufferImageCopies.push_back(bufferImageCopy);
	}
	vkCmdCopyBufferToImage(commandBuffer.commandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, arrayLayers, bufferImageCopies.data());

	commandBuffer.endAndSubmit();
	commandPool.destroy();
}
