#pragma once
#include "vulkan/vulkan.h"
#include "../../resources/Image.h"
#include "../../renderpasses/Framebuffer.h"
#include "../../renderpasses/RenderPass.h"
#include "../../pipelines/DescriptorSet.h"
#include "../../pipelines/ComputePipeline.h"
#include "../../pipelines/GraphicsPipeline.h"
#include "../../pipelines/Viewport.h"

#define TRANSMITTANCE_WIDTH 256
#define TRANSMITTANCE_HEIGHT 64

#define MULTI_SCATTERING_WIDTH 32
#define MULTI_SCATTERING_HEIGHT 32

#define SKY_VIEW_WIDTH 192
#define SKY_VIEW_HEIGHT 108

#define CAMERA_VOLUME_WIDTH 32
#define CAMERA_VOLUME_HEIGHT 32

struct Atmosphere {
	// Transmittance
	Image transmittanceImage;
	Viewport transmittanceViewport;
	Image dummyTransmittanceImage;
	GraphicsPipeline transmittanceGraphicsPipeline;
	DescriptorSet transmittanceDescriptorSet;
	RenderPass transmittanceRenderPass;
	Framebuffer transmittanceFramebuffer;

	// Sky view
	Image skyViewImage;
	Viewport skyViewViewport;
	GraphicsPipeline skyViewGraphicsPipeline;
	std::vector<DescriptorSet> skyViewDescriptorSets;
	RenderPass skyViewRenderPass;
	Framebuffer skyViewFramebuffer;

	// Camera volume
	Image cameraVolumeImage;
	Image cameraVolumeToSampleImage;
	Viewport cameraVolumeViewport;
	GraphicsPipeline cameraVolumeGraphicsPipeline;
	std::vector<DescriptorSet> cameraVolumeDescriptorSets;
	RenderPass cameraVolumeRenderPass;
	Framebuffer cameraVolumeFramebuffer;

	// Ray marching
	Viewport rayMarchingViewport;
	GraphicsPipeline rayMarchingGraphicsPipeline;
	std::vector<DescriptorSet> rayMarchingDescriptorSets;
	RenderPass rayMarchingRenderPass;
	Framebuffer rayMarchingFramebuffer;
	
	void init(Viewport fullscreenViewport);
	void destroy();
	void createResources(Viewport fullscreenViewport);
	void destroyResources();
	void draw(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex);
};