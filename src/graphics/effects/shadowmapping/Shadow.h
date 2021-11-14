#pragma once
#include "vulkan/vulkan.h"
#include "../../structs/RendererStructs.h"
#include "../../pipelines/Viewport.h"
#include "../../pipelines/GraphicsPipeline.h"
#include "../../resources/Buffer.h"
#include "../../resources/Image.h"
#include "../../renderpasses/Framebuffer.h"
#include "../../renderpasses/RenderPass.h"
#include "../../../utils/resources/BufferTools.h"
#include "../../../utils/resources/ImageTools.h"

struct Shadow {
	Viewport viewport;
	GraphicsPipeline opaqueGraphicsPipeline;
	GraphicsPipeline maskGraphicsPipeline;
	std::vector<Buffer> buffers;
	Image defaultShadow;
	std::vector<Image> directionalImages;
	std::vector<Image> spotImages;
	RenderPass renderPass;
	std::vector<Framebuffer> directionalFramebuffers;
	std::vector<Framebuffer> spotFramebuffers;

	void init();
	void destroy();
};