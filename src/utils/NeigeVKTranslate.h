#pragma once
#include "vulkan/vulkan.h"
#include <string>

struct NeigeVKTranslate {
	static std::string vkFormatToString(const VkFormat format);
	static std::string vkColorSpaceToString(VkColorSpaceKHR colorSpace);
	static std::string vkPresentModeToString(VkPresentModeKHR presentMode);
};