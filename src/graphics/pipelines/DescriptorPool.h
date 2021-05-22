#pragma once
#include "vulkan/vulkan.h"
#include "../commands/CommandBuffer.h"

struct DescriptorPool {
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	uint32_t remainingSets = 0;

	void destroy();
};