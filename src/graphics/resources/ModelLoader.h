#pragma once
#pragma warning(disable:4996)
#include "../../graphics/resources/Image.h"
#include "../../utils/NeigeDefines.h"
#include "../../graphics/structs/ModelStructs.h"
#include "../../graphics/structs/ShaderStructs.h"
#include "../../physics/AABB.h"
#include "../../../external/glm/glm/gtc/type_ptr.hpp"
#include "../../../external/glm/glm/gtx/quaternion.hpp"
#include "../../../external/glm/glm/gtx/string_cast.hpp"
#include "../../external/cgltf/cgltf.h"
#include "../../utils/resources/FileTools.h"
#include "ImageTools.h"
#include <vector>
#include <numeric>

struct ModelLoader {
	static void load(const std::string& filePath, std::vector<Vertex>* vertices, std::vector<uint32_t>* indices, std::vector<Mesh>* meshes);
	static void loadglTF(const std::string& filePath, std::vector<Vertex>* vertices, std::vector<uint32_t>* indices, std::vector<Mesh>* meshes);
	static void loadglTFNode(const std::string& filePath, cgltf_node* node, uint32_t* indexOffset, int32_t* modelVertexOffset, glm::mat4 modelMatrix, std::vector<Vertex>* vertices, std::vector<uint32_t>* indices, std::vector<Mesh>* meshes);
	static void loadglTFJoint(const std::string& filePath, cgltf_node* node, glm::mat4 globalTransform, glm::mat4 localTransform, Bone* hierarchy, std::vector<Bone>* boneList, std::vector<cgltf_node*> nodeList, std::vector<glm::mat4> inverseBindMatrixList);
	static int findTexture(std::string key);
	static int findMaterial(int diffuseIndex, int normalIndex, int metallicRoughnessIndex, int emissiveIndex, int occlusionIndex);
};