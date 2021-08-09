#pragma once
#include "vulkan/vulkan.h"
#include "../NeigeDefines.h"
#include "../../graphics/structs/RendererStructs.h"
#include <vector>
#include <iostream>
#include <algorithm>

// 256 MB
#define CHUNK_SIZE 268435456

struct Block {
	Block* prev;
	Block* next;

	VkDeviceSize offset;
	VkDeviceSize size;
	VkDeviceSize allocationId;
	bool inUse;
};

struct Chunk {
	VkDeviceSize id;
	VkDeviceMemory memory;
	int32_t type;
	Block* head;
	void* data;

	Chunk(VkDeviceSize chunkId, int32_t memoryType, VkDeviceSize size, bool isHostVisible);
	VkDeviceSize allocate(VkMemoryRequirements memRequirements, VkDeviceSize* allocationNumber, MemoryInfo* memoryInfo);
	void freeBlocks();
};

struct MemoryAllocator {
	std::vector<Chunk> chunks;
	VkDeviceSize allocationNumber = 1;

	void destroy();
	VkDeviceSize allocate(VkBuffer* bufferToAllocate, VkMemoryPropertyFlags flags, MemoryInfo* memoryInfo);
	VkDeviceSize allocate(VkImage* imageToAllocate, VkMemoryPropertyFlags flags, MemoryInfo* memoryInfo);
	void deallocate(VkDeviceSize chunkId, VkDeviceSize allocationId);
	int32_t findProperties(uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties);
	void memoryAnalyzer();
};