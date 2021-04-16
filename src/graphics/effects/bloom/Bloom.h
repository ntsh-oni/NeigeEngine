#pragma once
#include "vulkan/vulkan.hpp"
#include "../../resources/Image.h"
#include "../../renderpasses/Framebuffer.h"
#include "../../renderpasses/RenderPass.h"
#include "../../pipelines/DescriptorSet.h"
#include "../../pipelines/GraphicsPipeline.h"
#include "../../pipelines/Viewport.h"

#define BLOOMDOWNSCALE 4
#define BLURQUANTITY 1

struct Bloom {
	Viewport viewport;

	// Threshold and bloom
	Image thresholdImage;
	Image bloomImage;

	// Resize
	GraphicsPipeline resizeGraphicsPipeline;
	DescriptorSet resizeDescriptorSet;
	RenderPass resizeRenderPass;
	Framebuffer resizeFramebuffer;

	// Blur
	GraphicsPipeline blurGraphicsPipeline;
	DescriptorSet blurDescriptorSet;
	DescriptorSet backBlurDescriptorSet;
	Image blurredImage;
	RenderPass blurRenderPass;
	Framebuffer blurFramebuffer;
	Framebuffer backBlurFramebuffer;
	int horizontalBlur = 1;
	
	void init(Viewport fullscreenViewport);
	void destroy();
	void createResources(Viewport fullscreenViewport);
	void destroyResources();
	void draw(CommandBuffer* commandBuffer);
};