#pragma once
#include "vulkan/vulkan.h"
#include "../../utils/resources/ModelLoader.h"
#include "../../utils/structs/ModelStructs.h"
#include "../../utils/structs/ShaderStructs.h"
#include "../commands/CommandBuffer.h"
#include "../../utils/resources/BufferTools.h"
#include "../pipelines/GraphicsPipeline.h"
#include "../../graphics/resources/Buffer.h"
#include "../../physics/AABB.h"
#include <limits>
#include <vector>

struct Model {
	std::vector<Mesh> meshes;
	Buffer vertexBuffer;
	Buffer indexBuffer;
	bool gotOpaquePrimitives = false;
	bool gotMaskPrimitives = false;
	bool gotBlendPrimitives = false;
	AABB aabb = { glm::vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()), glm::vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()) };

	void init(std::string filePath);
	void destroy();
	void bindBuffers(CommandBuffer* commandBuffer);
	void drawOpaque(CommandBuffer* commandBuffer, GraphicsPipeline* opaqueGraphicsPipeline, bool bindTextures, bool culling);
	void drawMask(CommandBuffer* commandBuffer, GraphicsPipeline* maskGraphicsPipeline, bool bindTextures, bool culling, uint32_t pushConstantOffset);
	void drawBlend(CommandBuffer* commandBuffer, GraphicsPipeline* blendGraphicsPipeline, bool bindTextures, bool culling);
};