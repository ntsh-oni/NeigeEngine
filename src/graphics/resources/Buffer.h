#pragma once
#include "vulkan/vulkan.h"
#include "../structs/RendererStructs.h"

struct Buffer {
	VkBuffer buffer = VK_NULL_HANDLE;

	MemoryInfo memoryInfo = {};

	void destroy();
	void map(VkDeviceSize offset, VkDeviceSize size, void** data);
	void unmap();
};
