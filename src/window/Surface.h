#pragma once
#include "vulkan/vulkan.h"

struct Surface {
	VkSurfaceKHR surface = VK_NULL_HANDLE;

	void destroy();
};