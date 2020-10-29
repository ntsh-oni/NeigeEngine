#pragma once
#include "vulkan/vulkan.hpp"

enum struct AttachmentType {
	COLOR,
	DEPTH,
	SWAPCHAIN
};

struct RenderPassAttachment {
	AttachmentType type;
	VkAttachmentDescription description = {};

	RenderPassAttachment(AttachmentType attachmentType,
		VkFormat format,
		VkSampleCountFlagBits msaaSamples,
		VkAttachmentLoadOp loadOp,
		VkAttachmentStoreOp storeOp,
		VkAttachmentLoadOp stencilLoadOp,
		VkAttachmentStoreOp stencilStoreOp);
};