#pragma once
#include "vulkan/vulkan.h"
#include "../../utils/NeigeDefines.h"

struct Fence {
	VkFence fence;

	void init();
	void destroy();
	void wait();
	void reset();
};

