#pragma once
#include "vulkan/vulkan.h"
#include "../NeigeDefines.h"
#include "../structs/RendererStructs.h"
#include "../../graphics/commands/CommandBuffer.h"
#include "../../graphics/commands/CommandPool.h"

struct BufferTools {
	static void createBuffer(VkBuffer& buffer,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags memoryProperties,
		MemoryInfo* memoryInfo);
	static void createStagingBuffer(VkBuffer& buffer,
		VkDeviceSize size,
		MemoryInfo* memoryInfo);
	static void createUniformBuffer(VkBuffer& buffer,
		VkDeviceSize size,
		MemoryInfo* memoryInfo);
	static void createStorageBuffer(VkBuffer& buffer,
		VkDeviceSize size,
		MemoryInfo* memoryInfo);
	static void createIndirectBuffer(VkBuffer& buffer,
		VkDeviceSize size,
		MemoryInfo* memoryInfo);
	static void copyBuffer(VkBuffer srcBuffer,
		VkBuffer dstBuffer,
		VkDeviceSize size);
	static void copyToImage(VkBuffer srcBuffer,
		VkImage dstImage,
		uint32_t width,
		uint32_t height,
		uint32_t arrayLayers,
		uint64_t stride);
};

