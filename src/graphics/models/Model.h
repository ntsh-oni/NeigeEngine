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
	uint32_t opaqueDrawCount = 0;
	std::vector<VkDrawIndexedIndirectCommand> opaqueDrawIndirectCommands;
	std::vector<PerDraw> opaqueDrawIndirectInfos;
	Buffer opaqueDrawIndirectBuffer;
	Buffer opaqueDrawIndirectInfoBuffer;
	DescriptorSet opaqueDrawIndirectInfoDescriptorSet;
	uint32_t opaqueCulledDrawCount = 0;
	std::vector<Buffer> opaqueCulledDrawIndirectBuffers;
	std::vector<Buffer> opaqueCulledDrawIndirectInfoBuffers;
	std::vector<DescriptorSet> opaqueCulledDrawIndirectInfoDescriptorSets;

	bool gotMaskPrimitives = false;
	uint32_t maskDrawCount = 0;
	std::vector<VkDrawIndexedIndirectCommand> maskDrawIndirectCommands;
	std::vector<PerDraw> maskDrawIndirectInfos;
	Buffer maskDrawIndirectBuffer;
	Buffer maskDrawIndirectInfoBuffer;
	DescriptorSet maskDrawIndirectInfoDescriptorSet;
	uint32_t maskCulledDrawCount = 0;
	std::vector<Buffer> maskCulledDrawIndirectBuffers;
	std::vector<Buffer> maskCulledDrawIndirectInfoBuffers;
	std::vector<DescriptorSet> maskCulledDrawIndirectInfoDescriptorSets;

	bool gotBlendPrimitives = false;
	uint32_t blendDrawCount = 0;
	std::vector<VkDrawIndexedIndirectCommand> blendDrawIndirectCommands;
	std::vector<PerDraw> blendDrawIndirectInfos;
	Buffer blendDrawIndirectBuffer;
	Buffer blendDrawIndirectInfoBuffer;
	DescriptorSet blendDrawIndirectInfoDescriptorSet;
	uint32_t blendCulledDrawCount = 0;
	std::vector<Buffer> blendCulledDrawIndirectBuffers;
	std::vector<Buffer> blendCulledDrawIndirectInfoBuffers;
	std::vector<DescriptorSet> blendCulledDrawIndirectInfoDescriptorSets;

	AABB aabb = { glm::vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()), glm::vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()) };

	void init(std::string filePath);
	void destroy();
	void bindBuffers(CommandBuffer* commandBuffer);
	void drawOpaque(CommandBuffer* commandBuffer, GraphicsPipeline* opaqueGraphicsPipeline, bool bindTextures, uint32_t frameInFlightIndex, bool culling);
	void drawMask(CommandBuffer* commandBuffer, GraphicsPipeline* maskGraphicsPipeline, bool bindTextures, uint32_t frameInFlightIndex, bool culling, uint32_t pushConstantOffset);
	void drawBlend(CommandBuffer* commandBuffer, GraphicsPipeline* blendGraphicsPipeline, bool bindTextures, uint32_t frameInFlightIndex, bool culling);
};