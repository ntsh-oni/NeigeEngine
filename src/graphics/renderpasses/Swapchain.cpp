#include "Swapchain.h"
#include "../resources/ImageTools.h"
#include "../resources/RendererResources.h"
#include "../../window/WindowResources.h"

void Swapchain::init(uint32_t* swapchainSize) {
	swapchainSupport = physicalDevice.swapchainSupport(window.surface.surface);
	extent = swapchainSupport.extent(window.extent);
	surfaceFormat = swapchainSupport.surfaceFormat();
	presentMode = swapchainSupport.presentMode();
	uint32_t minImageCount = swapchainSupport.capabilities.minImageCount + 1;
	if (swapchainSupport.capabilities.maxImageCount > 0 && minImageCount > swapchainSupport.capabilities.maxImageCount) {
		minImageCount = swapchainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = window.surface.surface;
	swapchainCreateInfo.minImageCount = minImageCount;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	std::array<uint32_t, 2> swapchainQueueFamilies;
	if (physicalDevice.queueFamilyIndices.graphicsFamily.value() != physicalDevice.queueFamilyIndices.presentFamily.value()) {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainQueueFamilies = { physicalDevice.queueFamilyIndices.graphicsFamily.value(), physicalDevice.queueFamilyIndices.presentFamily.value() };
		swapchainCreateInfo.pQueueFamilyIndices = swapchainQueueFamilies.data();
	}
	else {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}
	swapchainCreateInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	VkSwapchainKHR oldSwapchain = swapchain;
	swapchainCreateInfo.oldSwapchain = oldSwapchain;
	NEIGE_VK_CHECK(vkCreateSwapchainKHR(logicalDevice.device, &swapchainCreateInfo, nullptr, &swapchain));

	NEIGE_VK_CHECK(vkGetSwapchainImagesKHR(logicalDevice.device, swapchain, &imageNumber, nullptr));
	*swapchainSize = imageNumber;
	framesInFlight = *swapchainSize < MAX_FRAMES_IN_FLIGHT ? *swapchainSize : MAX_FRAMES_IN_FLIGHT;
	images.resize(imageNumber);
	imageViews.resize(imageNumber);
	NEIGE_VK_CHECK(vkGetSwapchainImagesKHR(logicalDevice.device, swapchain, &imageNumber, images.data()));
	for (uint32_t i = 0; i < imageNumber; i++) {
		ImageTools::createImageView(&imageViews[i], images[i], 0, 1, 0, 1, VK_IMAGE_VIEW_TYPE_2D, surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void Swapchain::destroy() {
	destroyResources();
	vkDestroySwapchainKHR(logicalDevice.device, swapchain, nullptr);
}

void Swapchain::destroyResources() {
	for (uint32_t i = 0; i < imageNumber; i++) {
		vkDestroyImageView(logicalDevice.device, imageViews[i], nullptr);
	}
}

VkResult Swapchain::acquireNextImage(Semaphore* imageAvailableSemaphore, uint32_t* index) {
	return vkAcquireNextImageKHR(logicalDevice.device, swapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore->semaphore, VK_NULL_HANDLE, index);
}
