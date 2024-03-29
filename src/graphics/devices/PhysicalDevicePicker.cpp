#include "PhysicalDevicePicker.h"
#include "../resources/RendererResources.h"
#include "../../window/WindowResources.h"

void PhysicalDevicePicker::pick() {
	PhysicalDevice preferredDevice;
	VkPhysicalDeviceType preferredDeviceType = VK_PHYSICAL_DEVICE_TYPE_OTHER;
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance.instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		NEIGE_ERROR("Unable to find any GPU supporting Vulkan.");
	}
	std::vector<VkPhysicalDevice> enumerateDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance.instance, &deviceCount, enumerateDevices.data());
	std::vector<PhysicalDevice> devices;
	for (const VkPhysicalDevice device : enumerateDevices) {
		PhysicalDevice tmpPhysicalDevice;
		tmpPhysicalDevice.device = device;
		devices.push_back(tmpPhysicalDevice);
	}
	for (PhysicalDevice device : devices) {
		if (device.isSuitable(&window.surface)) {
			if (device.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				// GPU
				preferredDevice = device;
				preferredDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
			}
			else if (device.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
				// Chipset
				if (preferredDeviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
					preferredDevice = device;
					preferredDeviceType = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
				}
			}
			else if (device.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) {
				// Virtual GPU
				if (preferredDeviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
					preferredDevice = device;
					preferredDeviceType = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
				}
			}
		}
	}
	physicalDevice = preferredDevice;
	physicalDevice.getMaxUsableSampleCount();
	physicalDevice.findColorFormat();
	physicalDevice.findDepthFormat();
	NEIGE_ASSERT(physicalDevice.device != VK_NULL_HANDLE, "Unable to find a suitable GPU.");
	NEIGE_INFO(physicalDevice.properties.deviceName);
}
