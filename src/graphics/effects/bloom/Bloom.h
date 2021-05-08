#pragma once
#include "vulkan/vulkan.h"
#include "../../resources/Image.h"
#include "../../renderpasses/Framebuffer.h"
#include "../../renderpasses/RenderPass.h"
#include "../../pipelines/DescriptorSet.h"
#include "../../pipelines/GraphicsPipeline.h"
#include "../../pipelines/Viewport.h"

struct Bloom {
	Viewport viewport;

	// Threshold and bloom
	Image bloomImage;
	VkImageView bloomLod0ImageView;
	uint32_t mipLevels;
	std::vector<uint32_t> mipWidths;
	std::vector<uint32_t> mipHeights;
	float bloomThreshold = 1.5f;

	// Resize
	GraphicsPipeline resizeGraphicsPipeline;
	DescriptorSet resizeDescriptorSet;
	RenderPass resizeRenderPass;
	Framebuffer resizeFramebuffer;
	int bloomDownscale = 4;

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
	int blurSize = 9;
	
	void init(int downscale, float threshold, int size, Viewport fullscreenViewport);
	void destroy();
	void createResources(Viewport fullscreenViewport);
	void destroyResources();
	void draw(CommandBuffer* commandBuffer);
};