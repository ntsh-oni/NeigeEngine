#include "RenderPass.h"
#include "../resources/RendererResources.h"

void RenderPass::init(std::vector<RenderPassAttachment> attachments, std::vector<SubpassDependency> subpassDependencies) {
	for (size_t i = 0; i < attachments.size(); i++) {
		attachmentDescriptions.push_back(attachments[i].description);
		VkAttachmentReference reference = {};
		reference.attachment = attachments[i].type != AttachmentType::UNUSED ? static_cast<uint32_t>(i) : VK_ATTACHMENT_UNUSED;
		VkClearValue clearValue = {};
		switch (attachments[i].type) {
		case AttachmentType::COLOR:
			reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachmentReferences.push_back(reference);
			clearValue.color = { attachments[i].clearColorValue.r, attachments[i].clearColorValue.g, attachments[i].clearColorValue.b, attachments[i].clearColorValue.a };
			clearValues.push_back(clearValue);
			break;
		case AttachmentType::DEPTH:
			reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachmentReference = reference;
			clearValue.depthStencil = { attachments[i].clearDepthValue.depth, attachments[i].clearDepthValue.stencil };
			clearValues.push_back(clearValue);
			gotADepthAttachment = true;
			break;
		case AttachmentType::RESOLVE:
			reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			resolveAttachmentReferences.push_back(reference);
			break;
		case AttachmentType::UNUSED:
			reference.layout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachmentReferences.push_back(reference);
			clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };
			clearValues.push_back(clearValue);
		}
	}

	VkSubpassDescription subpassDescription = {};
	subpassDescription.flags = 0;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferences.size());
	subpassDescription.pColorAttachments = colorAttachmentReferences.data();
	subpassDescription.pResolveAttachments = resolveAttachmentReferences.data();
	subpassDescription.pDepthStencilAttachment = gotADepthAttachment ? &depthAttachmentReference : nullptr;

	std::vector<VkSubpassDependency> dependencies;
	for (size_t i = 0; i < subpassDependencies.size(); i++) {
		VkSubpassDependency subpassDepedency = {};
		subpassDepedency.srcSubpass = subpassDependencies[i].srcSubpass;
		subpassDepedency.dstSubpass = subpassDependencies[i].dstSubpass;
		subpassDepedency.srcStageMask = subpassDependencies[i].srcStageMask;
		subpassDepedency.dstStageMask = subpassDependencies[i].dstStageMask;
		subpassDepedency.srcAccessMask = subpassDependencies[i].srcAccessMask;
		subpassDepedency.dstAccessMask = subpassDependencies[i].dstAccessMask;
		subpassDepedency.dependencyFlags = subpassDependencies[i].dependencyFlags;
		dependencies.push_back(subpassDepedency);
	}

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = nullptr;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
	renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassCreateInfo.pDependencies = dependencies.data();
	NEIGE_VK_CHECK(vkCreateRenderPass(logicalDevice.device, &renderPassCreateInfo, nullptr, &renderPass));
}

void RenderPass::destroy() {
	vkDestroyRenderPass(logicalDevice.device, renderPass, nullptr);
}

void RenderPass::begin(CommandBuffer* commandBuffer, VkFramebuffer framebuffer, VkExtent2D extent) {
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = framebuffer;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = extent;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();
	vkCmdBeginRenderPass(commandBuffer->commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end(CommandBuffer* commandBuffer) {
	vkCmdEndRenderPass(commandBuffer->commandBuffer);
}