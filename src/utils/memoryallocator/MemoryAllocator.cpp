#include "MemoryAllocator.h"
#include "../../graphics/resources/RendererResources.h"

Chunk::Chunk(VkDeviceSize chunkId, int32_t memoryType, VkDeviceSize size, bool isHostVisible) {
	id = chunkId;
	type = memoryType;

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = size;
	allocInfo.memoryTypeIndex = memoryType;
	NEIGE_VK_CHECK(vkAllocateMemory(logicalDevice.device, &allocInfo, nullptr, &memory));

	// Map host visible memory
	if (isHostVisible) {
		NEIGE_VK_CHECK(vkMapMemory(logicalDevice.device, memory, 0, size, 0, &data));
	}

	Block* block = new Block();
	block->offset = 0;
	block->size = size;
	block->inUse = false;
	block->prev = nullptr;
	block->next = nullptr;
	block->allocationId = 0;

	head = block;
}

VkDeviceSize Chunk::allocate(VkMemoryRequirements memRequirements, VkDeviceSize* allocationNumber, MemoryInfo* memoryInfo) {
	Block* curr = head;
	while (curr) {
		if (!curr->inUse) {
			VkDeviceSize actualSize = curr->size;

			// Size of the block minus the alignment (wasted bytes)
			if (curr->offset % memRequirements.alignment != 0) {
				actualSize -= memRequirements.alignment - curr->offset % memRequirements.alignment;
			}

			// If we have enough space to allocate here
			if (actualSize >= memRequirements.size) {
				// Set the size of the block to be the size with alignment
				curr->size = actualSize;

				// Align offset
				if (curr->offset % memRequirements.alignment != 0) {
					curr->offset += memRequirements.alignment - curr->offset % memRequirements.alignment;
				}

				// If we have exactly enough space, no need to subdivide
				if (curr->size == memRequirements.size) {
					curr->inUse = true;
					curr->allocationId = (*allocationNumber)++;

					memoryInfo->chunkId = id;
					memoryInfo->offset = curr->offset;
					memoryInfo->allocationId = (*allocationNumber) - 1;
					memoryInfo->data = data;

					return curr->offset;
				}

				// Subdivide the block by creating a new one
				Block* newBlock = new Block();
				newBlock->inUse = false;
				newBlock->offset = curr->offset + memRequirements.size;
				newBlock->size = curr->size - memRequirements.size;
				newBlock->prev = curr;
				newBlock->next = curr->next;
				newBlock->allocationId = 0;

				// The size of the current block is now the size of the data
				curr->size = memRequirements.size;
				curr->inUse = true;
				if (curr->next) {
					curr->next->prev = newBlock;
				}
				curr->next = newBlock;
				curr->allocationId = (*allocationNumber)++;

				memoryInfo->chunkId = id;
				memoryInfo->offset = curr->offset;
				memoryInfo->allocationId = (*allocationNumber) - 1;
				memoryInfo->data = data;

				return curr->offset;
			}
		}
		curr = curr->next;
	}

	return -1;
}

void Chunk::freeBlocks() {
	Block* curr;
	while (head) {
		curr = head;
		head = head->next;
		delete curr;
	}
}

void MemoryAllocator::destroy() {
	for (Chunk chunk : chunks) {
		chunk.freeBlocks();
		vkFreeMemory(logicalDevice.device, chunk.memory, nullptr);
	}
}

VkDeviceSize MemoryAllocator::allocate(VkBuffer* bufferToAllocate, VkMemoryPropertyFlags flags, MemoryInfo* memoryInfo) {
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(logicalDevice.device, *bufferToAllocate, &memRequirements);
	int32_t properties = findProperties(memRequirements.memoryTypeBits, flags);

	// Look for the first block with enough space
	for (Chunk& chunk : chunks) {
		if (chunk.type == properties) {
			VkDeviceSize offset;
			offset = chunk.allocate(memRequirements, &allocationNumber, memoryInfo);

			if (offset != -1) {
				vkBindBufferMemory(logicalDevice.device, *bufferToAllocate, chunk.memory, offset);
				return allocationNumber - 1;
			}
		}
	}

	// No block has been found, create a new chunk
	VkDeviceSize allocationSize = std::max((VkDeviceSize)CHUNK_SIZE, memRequirements.size);
	Chunk newChunk = Chunk(static_cast<VkDeviceSize>(chunks.size()), properties, allocationSize, flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	// Add to this chunk
	VkDeviceSize offset;
	offset = newChunk.allocate(memRequirements, &allocationNumber, memoryInfo);

	if (offset == -1) {
		NEIGE_ERROR("Unable to allocate memory (buffer).");
	}

	vkBindBufferMemory(logicalDevice.device, *bufferToAllocate, newChunk.memory, offset);

	chunks.push_back(newChunk);

	MEMORY_INFO("New memory chunk allocated (number: " + std::to_string(chunks.size() - 1) + ", size: " + std::to_string(allocationSize) + ").");
	return allocationNumber - 1;
}

VkDeviceSize MemoryAllocator::allocate(VkImage* imageToAllocate, VkMemoryPropertyFlags flags, MemoryInfo* memoryInfo) {
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(logicalDevice.device, *imageToAllocate, &memRequirements);
	int32_t properties = findProperties(memRequirements.memoryTypeBits, flags);

	// Look for the first block with enough space
	for (Chunk& chunk : chunks) {
		if (chunk.type == properties) {
			VkDeviceSize offset;
			offset = chunk.allocate(memRequirements, &allocationNumber, memoryInfo);

			if (offset != -1) {
				vkBindImageMemory(logicalDevice.device, *imageToAllocate, chunk.memory, offset);
				return allocationNumber - 1;
			}
		}
	}

	// No block has been found, create a new chunk
	VkDeviceSize allocationSize = std::max((VkDeviceSize)CHUNK_SIZE, memRequirements.size);
	Chunk newChunk = Chunk(static_cast<VkDeviceSize>(chunks.size()), properties, allocationSize, flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	// Add to this chunk
	VkDeviceSize offset;
	offset = newChunk.allocate(memRequirements, &allocationNumber, memoryInfo);

	if (offset == -1) {
		NEIGE_ERROR("Unable to allocate memory (image).");
	}

	vkBindImageMemory(logicalDevice.device, *imageToAllocate, newChunk.memory, offset);

	chunks.push_back(newChunk);

	MEMORY_INFO("New memory chunk allocated (number: " + std::to_string(chunks.size() - 1) + ", size: " + std::to_string(allocationSize) + ").");
	return allocationNumber - 1;
}

void MemoryAllocator::deallocate(VkDeviceSize chunkId, VkDeviceSize allocationId) {
	Chunk* chunk = &chunks[chunkId];
	Block* curr = chunk->head;
	while (curr) {
		if (curr->inUse && curr->allocationId == allocationId) {
			curr->inUse = false;
			curr->allocationId = 0;

			// Blocks fusion
			// Fusion with previous block
			if (curr->prev && !curr->prev->inUse) {
				Block* prev = curr->prev;
				if (prev->prev) {
					prev->prev->next = curr;
					curr->prev = prev->prev;
				}
				else {
					curr->prev = nullptr;
					chunk->head = curr;
				}
				curr->offset = prev->offset;
				curr->size += prev->size;
				delete prev;
			}
			// Fusion with next block
			if (curr->next && !curr->next->inUse) {
				Block* next = curr->next;
				if (next->next) {
					next->next->prev = curr;
					curr->next = next->next;
				}
				else {
					curr->next = nullptr;
				}
				curr->size += next->size;
				delete next;
			}
			return;
		}
		curr = curr->next;
	}
	NEIGE_WARNING("Could not deallocate.");
}

int32_t MemoryAllocator::findProperties(uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties) {
	const uint32_t memoryCount = physicalDevice.memoryProperties.memoryTypeCount;
	for (uint32_t memoryIndex = 0; memoryIndex < memoryCount; memoryIndex++) {
		const uint32_t memoryTypeBits = (1 << memoryIndex);
		const bool isRequiredMemoryType = memoryTypeBitsRequirement & memoryTypeBits;

		const VkMemoryPropertyFlags properties = physicalDevice.memoryProperties.memoryTypes[memoryIndex].propertyFlags;
		const bool hasRequiredProperties = (properties & requiredProperties) == requiredProperties;

		if (isRequiredMemoryType && hasRequiredProperties) {
			return static_cast<int32_t>(memoryIndex);
		}
	}
	NEIGE_ERROR("Unable to find suitable memory type.");
}

void MemoryAllocator::memoryAnalyzer() {
	MEMORY_INFO("Showing all memory chunks:");
	for (size_t i = 0; i < chunks.size(); i++) {
		Chunk& chunk = chunks.at(i);
		std::cout << "Chunk " << chunk.id << ": " << physicalDevice.memoryProperties.memoryTypes[chunk.type].propertyFlags << std::endl;
		Block* curr = chunk.head;
		std::cout << "[";
		while (curr) {
			std::string blockInfo = "";
			blockInfo += curr->inUse ? " \033[32mO\033[39m " : " \033[31mX\033[39m ";
			blockInfo += "\033[95m" + std::to_string(curr->offset) + "\033[39m:\033[96m" + std::to_string(curr->size) + "\033[39m";
			blockInfo += curr->next ? " | " : " ";

			std::cout << blockInfo;

			curr = curr->next;
		}
		std::cout << "]" << std::endl << std::endl;
	}
}
