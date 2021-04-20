#pragma once
#include "vulkan/vulkan.h"

struct Semaphore {
	VkSemaphore semaphore;

	void init();
	void destroy();
};
