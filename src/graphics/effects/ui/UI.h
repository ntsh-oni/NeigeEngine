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

	GraphicsPipeline* currentGraphicsPipeline = nullptr;

	DescriptorSet cameraDescriptorSet;
	RenderPass renderPass;
	std::vector<Framebuffer> framebuffers;

	Buffer cameraBuffer;

	GraphicsPipeline spriteGraphicsPipeline;
	DescriptorPool spritesDescriptorPool;
	VkDescriptorSetLayout spritesDescriptorSetLayout;

	GraphicsPipeline textGraphicsPipeline;
	DescriptorPool fontsDescriptorPool;
	VkDescriptorSetLayout fontsDescriptorSetLayout;


	void init(Viewport fullscreenViewport);
	void destroy();
	void createResources(Viewport fullscreenViewport);
	void destroyResources();
	void updateSpriteDescriptorSet(uint32_t frameInFlightIndex);
	void updateFontDescriptorSet(uint32_t frameInFlightIndex);
	void draw(CommandBuffer* commandBuffer, uint32_t framebufferIndex);
	void drawSprite(CommandBuffer* commandBuffer, Sprite sprite);
	void drawText(CommandBuffer* commandBuffer, Text text);
};