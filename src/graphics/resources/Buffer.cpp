#include "Buffer.h"
#include "../resources/RendererResources.h"

void Buffer::destroy() {
	memoryAllocator.deallocate(memoryInfo.chunkId, memoryInfo.allocationId);
	vkDestroyBuffer(logicalDevice.device, buffer, nullptr);
	buffer = VK_NULL_HANDLE;
}