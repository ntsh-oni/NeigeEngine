#pragma once
#include "vulkan/vulkan.h"
#include "../NeigeDefines.h"
#include "../structs/RendererStructs.h"
#include "BufferTools.h"
#include "FileTools.h"
#include "../../graphics/resources/Buffer.h"
#include "../../graphics/commands/CommandBuffer.h"
#include "../../graphics/commands/CommandPool.h"
#include <array>

struct ImageTools {
	static void createImage(VkImage* image,
		uint32_t arrayLayers,
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels,
		VkSampleCountFlagBits msaaSamples,
		VkFormat format,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags memoryProperties,
		MemoryInfo* memoryInfo);
	static void createImageView(VkImageView* imageView,
		VkImage image,
		uint32_t baseArrayLayer,
		uint32_t arrayLayers,
		uint32_t baseMipLevel,
		uint32_t mipLevels,
		VkImageViewType viewType,
		VkFormat format,
		VkImageAspectFlags aspectFlags);
	static void createImageSampler(VkSampler* sampler,
		uint32_t mipLevels,
		VkFilter filter,
		VkSamplerAddressMode addressMode,
		VkBorderColor borderColor,
		VkCompareOp compareOp);
	static void loadImage(const std::string& filePath,
		VkImage* imageDestination,
		VkFormat format,
		uint32_t* mipLevels,
		MemoryInfo* memoryInfo);
	static void loadHDREnvmap(const std::string& filePath,
		VkImage* imageDestination,
		VkFormat format,
		MemoryInfo* memoryInfo);
	static void loadColor(float* color,
		VkImage* imageDestination,
		VkFormat format,
		uint32_t* mipLevels,
		MemoryInfo* memoryInfo);
	static void loadColorArray(float* colors,
		VkImage* imageDestination,
		uint32_t width,
		uint32_t height,
		VkFormat format,
		uint32_t* mipLevels,
		MemoryInfo* memoryInfo);
	static void loadColorForEnvmap(float* color,
		VkImage* imageDestination,
		VkFormat format,
		uint32_t* mipLevels,
		MemoryInfo* memoryInfo);
	static void loadValue(float value,
		VkImage* imageDestination,
		VkFormat format,
		uint32_t* mipLevels,
		MemoryInfo* memoryInfo);
	static void transitionLayout(VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		uint32_t mipLevels,
		uint32_t arrayLayers);
	static void generateMipmaps(VkImage image,
		VkFormat format,
		int32_t texelWidth,
		int32_t texelHeight,
		uint32_t mipLevels,
		uint32_t arrayLayers);
};