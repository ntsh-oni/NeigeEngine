#pragma once
#include "vulkan/vulkan.hpp"
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

#define SSAODOWNSCALE 4
#define SSAOSAMPLES 64

struct SSAO {
	Viewport viewport;

	// Depth to positions and normals
	GraphicsPipeline depthToPositionsGraphicsPipeline;
	GraphicsPipeline depthToNormalsGraphicsPipeline;
	std::vector<DescriptorSet> depthToPositionsDescriptorSets;
	std::vector<DescriptorSet> depthToNormalsDescriptorSets;
	Image depthToPositionsImage;
	Image depthToNormalsImage;
	RenderPass depthToPositionsRenderPass;
	RenderPass depthToNormalsRenderPass;
	std::vector<Framebuffer> depthToPositionsFramebuffers;
	std::vector<Framebuffer> depthToNormalsFramebuffers;

	// Sample kernel
	Buffer sampleKernel;
	Image randomTexture;

	// SSAO
	GraphicsPipeline ssaoGraphicsPipeline;
	std::vector<DescriptorSet> ssaoDescriptorSets;
	Image ssaoImage;
	RenderPass ssaoRenderPass;
	std::vector<Framebuffer> ssaoFramebuffers;

	// SSAO blurred
	GraphicsPipeline ssaoBlurredGraphicsPipeline;
	DescriptorSet ssaoBlurredDescriptorSet;
	Image ssaoBlurredImage;
	RenderPass ssaoBlurredRenderPass;
	std::vector<Framebuffer> ssaoBlurredFramebuffers;

	void init(Viewport fullscreenViewport);
	void destroy();
	void createResources(Viewport fullscreenViewport);
	void destroyResources();
	void createRandomTexture();
	void draw(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex);
};