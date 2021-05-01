#pragma once
#include "vulkan/vulkan.h"
#include "../../external/glm/glm/glm.hpp"
#include "../../graphics/pipelines/DescriptorSet.h"
#include "../../graphics/pipelines/GraphicsPipeline.h"
#include "../../graphics/resources/Buffer.h"
#include <vector>
#include <unordered_map>

// Model AABB
struct AABB {
	float minX;
	float maxX;
	float minY;
	float maxY;
	float minZ;
	float maxZ;
};

// Model primitive
struct Primitive {
	uint32_t firstIndex;
	uint32_t indexCount;
	int32_t vertexOffset;
	uint64_t materialIndex;
	AABB aabb;
};

// Mesh bone
struct Bone {
	glm::mat4 transformation;
	glm::mat4 inverseBindMatrix;
	std::vector<Bone> children;
};

// Model mesh
struct Mesh {
	uint32_t indexOffset;
	int32_t vertexOffset;
	std::vector<Primitive> opaquePrimitives;
	std::vector<Primitive> maskPrimitives;
	std::vector<float> alphaCutoffs;
	std::vector<Primitive> blendPrimitives;
	Bone skeleton;
	std::vector<Bone> boneList;
	Buffer boneBuffer;
};