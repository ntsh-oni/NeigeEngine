#include "Buffer.h"
#include "../resources/RendererResources.h"

void Buffer::destroy() {
	memoryAllocator.deallocate(memoryInfo.chunkId, memoryInfo.allocationId);
	vkDestroyBuffer(logicalDevice.device, buffer, nullptr);
	buffer = VK_NULL_HANDLE;
}

void Buffer::map(VkDeviceSize offset, VkDeviceSize size, void** data) {
	vkMapMemory(logicalDevice.device, memoryAllocator.chunks[memoryInfo.chunkId].memory, memoryInfo.offset, size, 0, data);
}

void Buffer::unmap() {
	vkUnmapMemory(logicalDevice.device, memoryAllocator.chunks[memoryInfo.chunkId].memory);
}
