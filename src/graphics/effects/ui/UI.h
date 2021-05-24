#pragma once
#include "../../../../external/glm/glm/glm.hpp"
#include "../../commands/CommandBuffer.h"
#include "../../pipelines/DescriptorSet.h"
#include "../../pipelines/GraphicsPipeline.h"
#include "../../pipelines/Viewport.h"
#include "../../renderpasses/Framebuffer.h"
#include "../../renderpasses/RenderPass.h"
#include "../../resources/UIResources.h"
#include "../../structs/ShaderStructs.h"
#include "../../structs/UIStructs.h"
#include "../../../utils/resources/BufferTools.h"
#include <string>
#include <vector>

struct UI {
	Viewport viewport;

	GraphicsPipeline graphicsPipeline;
	DescriptorSet cameraDescriptorSet;
	DescriptorPool fontsDescriptorPool;
	VkDescriptorSetLayout fontsDescriptorSetLayout;
	RenderPass renderPass;

	Buffer cameraBuffer;

	void init(Viewport fullscreenViewport, bool enableFXAA);
	void destroy();
	void createResources(Viewport fullscreenViewport, bool enableFXAA);
	void updateFontDescriptorSet(uint32_t frameInFlightIndex);
	void draw(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex);
	void drawText(CommandBuffer* commandBuffer, Text text);
};