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
	void drawOpaque(CommandBuffer* commandBuffer, GraphicsPipeline* opaqueGraphicsPipeline, uint32_t frameInFlightIndex, bool bindTextures);
	void drawTransparent(CommandBuffer* commandBuffer, GraphicsPipeline* transparentGraphicsPipeline, uint32_t frameInFlightIndex, bool bindTextures);
	void createOpaqueDescriptorSets(GraphicsPipeline* opaqueGraphicsPipeline);
	void createTransparentDescriptorSets(GraphicsPipeline* transparentGraphicsPipeline);
};