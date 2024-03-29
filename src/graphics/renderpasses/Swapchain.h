#pragma once
#include "vulkan/vulkan.h"
#include "../structs/RendererStructs.h"
#include "../sync/Semaphore.h"
#include <array>

struct Swapchain {
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	SwapchainSupport swapchainSupport;
	VkExtent2D extent;
	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;
	uint32_t imageNumber;
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;

	void init(uint32_t* swapchainSize);
	void destroy();
	void destroyResources();
	VkResult acquireNextImage(Semaphore* imageAvailableSemaphore, uint32_t* index);
};