#pragma once
#include "vulkan/vulkan.h"
#include "../../utils/NeigeDefines.h"
#include "../../utils/structs/RendererStructs.h"
#include <limits>
#include <string>

struct Instance {
	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

	void init(const std::string& applicationName, uint32_t engineVersion, const std::vector<const char*>& windowExtensions);
	void destroy();
	void createDebugMessenger();
	void destroyDebugMessenger();
};