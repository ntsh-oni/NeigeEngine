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
	Buffer opaqueDrawCountBuffer;
	std::vector<VkDrawIndexedIndirectCommand> opaqueDrawIndirectCommands;
	std::vector<PerDraw> opaqueDrawIndirectInfos;
	Buffer opaqueDrawIndirectBuffer;
	Buffer opaqueDrawIndirectInfoBuffer;
	Buffer opaqueAABBBuffer;
	DescriptorSet opaqueFrustumCullingDescriptorSet;
	DescriptorSet opaqueDrawIndirectInfoDescriptorSet;

	bool gotMaskPrimitives = false;

	uint32_t maskDrawCount = 0;
	Buffer maskDrawCountBuffer;
	std::vector<VkDrawIndexedIndirectCommand> maskDrawIndirectCommands;
	std::vector<PerDraw> maskDrawIndirectInfos;
	Buffer maskDrawIndirectBuffer;
	Buffer maskDrawIndirectInfoBuffer;
	Buffer maskAABBBuffer;
	DescriptorSet maskFrustumCullingDescriptorSet;
	DescriptorSet maskDrawIndirectInfoDescriptorSet;

	bool gotBlendPrimitives = false;

	uint32_t blendDrawCount = 0;
	Buffer blendDrawCountBuffer;
	std::vector<VkDrawIndexedIndirectCommand> blendDrawIndirectCommands;
	std::vector<PerDraw> blendDrawIndirectInfos;
	Buffer blendDrawIndirectBuffer;
	Buffer blendDrawIndirectInfoBuffer;
	Buffer blendAABBBuffer;
	DescriptorSet blendFrustumCullingDescriptorSet;
	DescriptorSet blendDrawIndirectInfoDescriptorSet;

	AABB aabb = { glm::vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()), 0.0f, glm::vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()), 0.0f };

	void init(std::string filePath);
	void destroy();
	void bindBuffers(CommandBuffer* commandBuffer);
	void cullOpaque(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex);
	void cullMask(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex);
	void cullBlend(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex);
};