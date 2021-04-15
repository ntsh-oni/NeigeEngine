#pragma once
#include "vulkan/vulkan.hpp"
#include "../../utils/resources/ModelLoader.h"
#include "../../utils/structs/ModelStructs.h"
#include "../../utils/structs/ShaderStructs.h"
#include "../commands/CommandBuffer.h"
#include "../../utils/resources/BufferTools.h"
#include "../pipelines/GraphicsPipeline.h"
#include "../../graphics/resources/Buffer.h"
#include <vector>

struct Model {
	std::vector<Mesh> meshes;
	Buffer vertexBuffer;
	Buffer indexBuffer;

	void init(std::string filePath);
	void destroy();
	void bindBuffers(CommandBuffer* commandBuffer);
	void drawOpaque(CommandBuffer* commandBuffer, GraphicsPipeline* opaqueGraphicsPipeline, uint32_t frameInFlightIndex, bool bindTextures);
	void drawMask(CommandBuffer* commandBuffer, GraphicsPipeline* maskGraphicsPipeline, uint32_t frameInFlightIndex, bool bindTextures, uint32_t pushConstantOffset);
	void drawBlend(CommandBuffer* commandBuffer, GraphicsPipeline* blendGraphicsPipeline, uint32_t frameInFlightIndex, bool bindTextures);
	void createOpaqueDescriptorSets(GraphicsPipeline* opaqueGraphicsPipeline);
	void createMaskDescriptorSets(GraphicsPipeline* maskGraphicsPipeline);
	void createDiscardMaskDescriptorSets(GraphicsPipeline* discardGraphicsPipeline);
	void createBlendDescriptorSets(GraphicsPipeline* blendGraphicsPipeline);
};