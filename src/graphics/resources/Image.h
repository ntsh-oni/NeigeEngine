#pragma once
#include "vulkan/vulkan.h"
#include "../../utils/NeigeDefines.h"
#include "../structs/RendererStructs.h"

struct Image {
	VkImage image = VK_NULL_HANDLE;
	VkImageView imageView = VK_NULL_HANDLE;
	uint32_t width;
	uint32_t height;
	uint32_t mipmapLevels;

	MemoryInfo memoryInfo = {};

	void destroy();
};