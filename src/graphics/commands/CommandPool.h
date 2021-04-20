#pragma once
#include "vulkan/vulkan.h"
#include "../../utils/NeigeDefines.h"

struct CommandPool {
	VkCommandPool commandPool = VK_NULL_HANDLE;

	void init();
	void destroy();
	void reset();
};
