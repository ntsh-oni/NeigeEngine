#pragma once
#include "vulkan/vulkan.hpp"

enum struct AttachmentType {
	COLOR,
	DEPTH,
	RESOLVE
};

struct ClearColorValue {
	float r;
	float g;
	float b;
	float a;

	ClearColorValue(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
};

struct ClearDepthValue {
	float depth;
	uint32_t stencil;

	ClearDepthValue(float depth, uint32_t stencil) : depth(depth), stencil(stencil) {}
};

struct RenderPassAttachment {
	AttachmentType type;
	VkAttachmentDescription description = {};
	ClearColorValue clearColorValue = {0.0f, 0.0f, 0.0f, 0.0f};
	ClearDepthValue clearDepthValue = {1.0f , 0};

	RenderPassAttachment(AttachmentType attachmentType,
		VkFormat format,
		VkSampleCountFlagBits msaaSamples,
		VkAttachmentLoadOp loadOp,
		VkAttachmentStoreOp storeOp,
		VkAttachmentLoadOp stencilLoadOp,
		VkAttachmentStoreOp stencilStoreOp,
		VkImageLayout initialLayout,
		VkImageLayout finalLayout,
		ClearColorValue clearValue);

	RenderPassAttachment(AttachmentType attachmentType,
		VkFormat format,
		VkSampleCountFlagBits msaaSamples,
		VkAttachmentLoadOp loadOp,
		VkAttachmentStoreOp storeOp,
		VkAttachmentLoadOp stencilLoadOp,
		VkAttachmentStoreOp stencilStoreOp,
		VkImageLayout initialLayout,
		VkImageLayout finalLayout,
		ClearDepthValue clearValue);
};