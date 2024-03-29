#pragma once
#include "vulkan/vulkan.h"
#include "../../../../external/glm/glm/glm.hpp"
#include "../../commands/CommandBuffer.h"
#include "../../resources/Buffer.h"
#include "../../resources/Image.h"
#include "../../renderpasses/Framebuffer.h"
#include "../../renderpasses/RenderPass.h"
#include "../../pipelines/DescriptorSet.h"
#include "../../pipelines/GraphicsPipeline.h"
#include "../../pipelines/Viewport.h"
#include <numeric>
#include <vector>
#include <random>

#define SSAOSAMPLES 64

struct SSAO {
	Viewport viewport;

	int ssaoDownscale = 3;

	// Depth to positions and normals
	GraphicsPipeline depthToPositionsGraphicsPipeline;
	GraphicsPipeline depthToNormalsGraphicsPipeline;
	std::vector<DescriptorSet> depthToPositionsDescriptorSets;
	std::vector<DescriptorSet> depthToNormalsDescriptorSets;
	Image depthToPositionsImage;
	Image depthToNormalsImage;
	RenderPass depthToPositionsRenderPass;
	RenderPass depthToNormalsRenderPass;
	Framebuffer depthToPositionsFramebuffer;
	Framebuffer depthToNormalsFramebuffer;

	// Sample kernel
	Buffer sampleKernel;
	Image randomTexture;

	// SSAO
	GraphicsPipeline ssaoGraphicsPipeline;
	std::vector<DescriptorSet> ssaoDescriptorSets;
	Image ssaoImage;
	RenderPass ssaoRenderPass;
	Framebuffer ssaoFramebuffer;

	// SSAO blurred
	GraphicsPipeline ssaoBlurredGraphicsPipeline;
	DescriptorSet ssaoBlurredDescriptorSet;
	Image ssaoBlurredImage;
	RenderPass ssaoBlurredRenderPass;
	Framebuffer ssaoBlurredFramebuffer;

	void init(int downscale, Viewport fullscreenViewport);
	void destroy();
	void createResources(Viewport fullscreenViewport);
	void destroyResources();
	void createRandomTexture();
	void draw(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex);
};