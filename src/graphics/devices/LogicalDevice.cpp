#include "LogicalDevice.h"
#include "../resources/RendererResources.h"

void LogicalDevice::init() {
	// Queue family indices
	std::set<uint32_t> uniqueQueueFamilies = { physicalDevice.queueFamilyIndices.graphicsFamily.value(),
	physicalDevice.queueFamilyIndices.computeFamily.value(),
	physicalDevice.queueFamilyIndices.presentFamily.value() };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
		deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfo.pNext = nullptr;
		deviceQueueCreateInfo.flags = 0;
		deviceQueueCreateInfo.queueFamilyIndex = queueFamily;
		deviceQueueCreateInfo.queueCount = 1;
		deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(deviceQueueCreateInfo);
	}

	// Physical device features
	VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
	physicalDeviceFeatures.fillModeNonSolid = VK_TRUE;
	physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
	physicalDeviceFeatures.sampleRateShading = VK_TRUE;
	physicalDeviceFeatures.independentBlend = VK_TRUE;
	physicalDeviceFeatures.multiDrawIndirect = VK_TRUE;

	VkPhysicalDeviceDescriptorIndexingFeatures physicalDeviceDescriptorIndexingFeatures = {};
	physicalDeviceDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
	physicalDeviceDescriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	physicalDeviceDescriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
	physicalDeviceDescriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
	
	// Logical device
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = &physicalDeviceDescriptorIndexingFeatures;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	if (NEIGE_DEBUG) {
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(debugExplicitLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = debugExplicitLayers.data();
	}
	else {
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(releaseExplicitLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = releaseExplicitLayers.data();
	}
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vulkanExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = vulkanExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
	NEIGE_VK_CHECK(vkCreateDevice(physicalDevice.device, &deviceCreateInfo, nullptr, &device));

	// Queues
	vkGetDeviceQueue(device, physicalDevice.queueFamilyIndices.graphicsFamily.value(), 0, &queues.graphicsQueue);
	vkGetDeviceQueue(device, physicalDevice.queueFamilyIndices.computeFamily.value(), 0, &queues.computeQueue);
	vkGetDeviceQueue(device, physicalDevice.queueFamilyIndices.presentFamily.value(), 0, &queues.presentQueue);
}

void LogicalDevice::destroy() {
	vkDestroyDevice(device, nullptr);
}

void LogicalDevice::wait() {
	NEIGE_VK_CHECK(vkDeviceWaitIdle(device));
}
