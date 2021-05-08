#pragma once
#include "vulkan/vulkan.h"
#include "../../../utils/structs/ShaderStructs.h"
#include "../../pipelines/DescriptorSet.h"
#include "../../pipelines/ComputePipeline.h"

struct FrustumCulling {
	ComputePipeline computePipeline;

	void init();
	void destroy();
	void drawCountsReset(CommandBuffer* commandBuffer, const std::vector<VkBuffer>& drawCountBuffers);
	void computeIndirectFragmentBarrier(CommandBuffer* commandBuffer, const std::vector<VkBuffer>& indirectBuffers, const std::vector<VkBuffer>& drawCountBuffers, const std::vector<VkBuffer>& perDrawBuffers, const std::vector<uint32_t>& drawCounts);
};