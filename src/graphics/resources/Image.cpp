#include "Image.h"
#include "ImageTools.h"
#include "../resources/RendererResources.h"

void Image::destroy() {
	if (imageView != VK_NULL_HANDLE) {
		vkDestroyImageView(logicalDevice.device, imageView, nullptr);
		imageView = VK_NULL_HANDLE;
	}
	if (image != VK_NULL_HANDLE) {
		memoryAllocator.deallocate(memoryInfo.chunkId, memoryInfo.allocationId);
		vkDestroyImage(logicalDevice.device, image, nullptr);
		image = VK_NULL_HANDLE;
	}
}
