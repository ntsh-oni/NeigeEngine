#pragma once
#include "vulkan/vulkan.h"
#include "../../utils/NeigeDefines.h"
#include "../structs/RendererStructs.h"
#include <set>

struct LogicalDevice {
	VkDevice device = VK_NULL_HANDLE;
	Queues queues;

	void init();
	void destroy();
	void wait();
};