#include "DescriptorPool.h"
#include "../resources/RendererResources.h"

void DescriptorPool::destroy() {
	if (descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(logicalDevice.device, descriptorPool, nullptr);
		descriptorPool = VK_NULL_HANDLE;
		remainingSets = 0;
	}
}