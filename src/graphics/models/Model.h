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
	std::vector<VkBuffer> indirectBuffers;
	std::vector<VkBuffer> drawCountBuffers;
	std::vector<VkBuffer> perDrawBuffers;

	bool gotOpaquePrimitives = false;

	uint32_t opaqueDrawCount = 0;
	Buffer opaqueDrawCountBuffer;
	std::vector<VkDrawIndexedIndirectCommand> opaqueDrawIndirectCommands;
	std::vector<PerDraw> opaqueDrawIndirectInfos;
	Buffer opaqueDrawIndirectBuffer;
	Buffer opaqueDrawIndirectInfoBuffer;
	DescriptorSet opaqueDrawIndirectInfoDescriptorSet;

	uint32_t opaqueCulledDrawCount = 0;
	Buffer opaqueCulledDrawCountBuffer;
	Buffer opaqueCulledDrawIndirectBuffer;
	Buffer opaqueCulledDrawIndirectInfoBuffer;
	DescriptorSet opaqueCulledDrawIndirectInfoDescriptorSet;
	std::vector<Buffer> opaqueAABBBuffers;
	std::vector<DescriptorSet> opaqueFrustumCullingDescriptorSets;

	bool gotMaskPrimitives = false;

	uint32_t maskDrawCount = 0;
	Buffer maskDrawCountBuffer;
	std::vector<VkDrawIndexedIndirectCommand> maskDrawIndirectCommands;
	std::vector<PerDraw> maskDrawIndirectInfos;
	Buffer maskDrawIndirectBuffer;
	Buffer maskDrawIndirectInfoBuffer;
	DescriptorSet maskDrawIndirectInfoDescriptorSet;

	uint32_t maskCulledDrawCount = 0;
	Buffer maskCulledDrawCountBuffer;
	Buffer maskCulledDrawIndirectBuffer;
	Buffer maskCulledDrawIndirectInfoBuffer;
	DescriptorSet maskCulledDrawIndirectInfoDescriptorSet;
	std::vector<Buffer> maskAABBBuffers;
	std::vector<DescriptorSet> maskFrustumCullingDescriptorSets;

	bool gotBlendPrimitives = false;

	uint32_t blendDrawCount = 0;
	Buffer blendDrawCountBuffer;
	std::vector<VkDrawIndexedIndirectCommand> blendDrawIndirectCommands;
	std::vector<PerDraw> blendDrawIndirectInfos;
	Buffer blendDrawIndirectBuffer;
	Buffer blendDrawIndirectInfoBuffer;
	DescriptorSet blendDrawIndirectInfoDescriptorSet;

	uint32_t blendCulledDrawCount = 0;
	Buffer blendCulledDrawCountBuffer;
	Buffer blendCulledDrawIndirectBuffer;
	Buffer blendCulledDrawIndirectInfoBuffer;
	DescriptorSet blendCulledDrawIndirectInfoDescriptorSet;
	std::vector<Buffer> blendAABBBuffers;
	std::vector<DescriptorSet> blendFrustumCullingDescriptorSets;

	AABB aabb = { glm::vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()), 0.0f, glm::vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()), 0.0f };

	void init(std::string filePath);
	void destroy();
	void bindBuffers(CommandBuffer* commandBuffer);
	void cullOpaque(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex);
	void cullMask(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex);
	void cullBlend(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex);
	void drawOpaque(CommandBuffer* commandBuffer, GraphicsPipeline* opaqueGraphicsPipeline, bool bindTextures, uint32_t frameInFlightIndex, bool culling);
	void drawMask(CommandBuffer* commandBuffer, GraphicsPipeline* maskGraphicsPipeline, bool bindTextures, uint32_t frameInFlightIndex, bool culling, uint32_t pushConstantOffset);
	void drawBlend(CommandBuffer* commandBuffer, GraphicsPipeline* blendGraphicsPipeline, bool bindTextures, uint32_t frameInFlightIndex, bool culling);
};