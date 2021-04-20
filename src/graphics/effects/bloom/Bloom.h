#pragma once
#include "vulkan/vulkan.h"
#include "../../resources/Image.h"
#include "../../renderpasses/Framebuffer.h"
#include "../../renderpasses/RenderPass.h"
#include "../../pipelines/DescriptorSet.h"
#include "../../pipelines/GraphicsPipeline.h"
#include "../../pipelines/Viewport.h"

#define BLOOMDOWNSCALE 4

struct Bloom {
	Viewport viewport;

	// Threshold and bloom
	Image bloomImage;
	VkImageView bloomLod0ImageView;
	uint32_t mipLevels;
	std::vector<uint32_t> mipWidths;
	std::vector<uint32_t> mipHeights;

	// Resize
	GraphicsPipeline resizeGraphicsPipeline;
	DescriptorSet resizeDescriptorSet;
	RenderPass resizeRenderPass;
	Framebuffer resizeFramebuffer;

	// Blur
	Viewport blurViewport;
	GraphicsPipeline blurGraphicsPipeline;
	std::vector<DescriptorSet> blurDescriptorSets;
	std::vector<DescriptorSet> backBlurDescriptorSets;
	Image blurredImage;
	RenderPass blurRenderPass;
	std::vector<Framebuffer> blurFramebuffers;
	std::vector<Framebuffer> backBlurFramebuffers;
	std::vector<VkImageView> blurImageViews;
	std::vector<VkImageView> backBlurImageViews;
	int horizontalBlur = 1;
	
	void init(Viewport fullscreenViewport);
	void destroy();
	void createResources(Viewport fullscreenViewport);
	void destroyResources();
	void draw(CommandBuffer* commandBuffer);
};