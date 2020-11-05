#pragma once
#include "vulkan/vulkan.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "../../external/glm/glm/glm.hpp"
#include "../NeigeDefines.h"

#define MAX_DIR_LIGHTS 10
#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS 10

enum struct ShaderType {
	VERTEX,
	FRAGMENT,
	TESSELATION_CONTROL,
	TESSELATION_EVALUATION,
	GEOMETRY,
	COMPUTE
};

enum struct Topology {
	TRIANGLE_LIST,
	TRIANGLE_STRIP,
	LINE_STRIP,
	POINT_LIST,
	WIREFRAME
};

enum struct LightType {
	DIRECTIONAL,
	POINT,
	SPOT
};

// Material
struct Material {
	std::string diffuseKey = "";
	std::string normalKey = "";
	std::string metallicRoughnessKey = "";
	std::string occlusionKey = "";
};

// Vertex
struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
	glm::vec3 color;
	glm::vec3 tangent;

	static VkVertexInputBindingDescription getInputBindingDescription() {
		VkVertexInputBindingDescription inputBindingDescription = {};
		inputBindingDescription.binding = 0;
		inputBindingDescription.stride = sizeof(Vertex);
		inputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return inputBindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> getInputAttributeDescriptions(std::vector<std::string> inputVariables) {
		std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions;

		for (size_t i = 0; i < inputVariables.size(); i++) {
			if (inputVariables[i] == "position") {
				VkVertexInputAttributeDescription positionAttribute = {};
				positionAttribute.binding = 0;
				positionAttribute.location = 0;
				positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
				positionAttribute.offset = offsetof(Vertex, position);
				inputAttributeDescriptions.push_back(positionAttribute);
			}
			else if (inputVariables[i] == "normal") {
				VkVertexInputAttributeDescription normalAttribute = {};
				normalAttribute.binding = 0;
				normalAttribute.location = 1;
				normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
				normalAttribute.offset = offsetof(Vertex, normal);
				inputAttributeDescriptions.push_back(normalAttribute);
			}
			else if (inputVariables[i] == "uv") {
				VkVertexInputAttributeDescription uvAttribute = {};
				uvAttribute.binding = 0;
				uvAttribute.location = 2;
				uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
				uvAttribute.offset = offsetof(Vertex, uv);
				inputAttributeDescriptions.push_back(uvAttribute);
			}
			else if (inputVariables[i] == "color") {
				VkVertexInputAttributeDescription colorAttribute = {};
				colorAttribute.binding = 0;
				colorAttribute.location = 3;
				colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
				colorAttribute.offset = offsetof(Vertex, color);
				inputAttributeDescriptions.push_back(colorAttribute);
			}
			else if (inputVariables[i] == "tangent") {
				VkVertexInputAttributeDescription tangentAttribute = {};
				tangentAttribute.binding = 0;
				tangentAttribute.location = 4;
				tangentAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
				tangentAttribute.offset = offsetof(Vertex, tangent);
				inputAttributeDescriptions.push_back(tangentAttribute);
			}
		}

		return inputAttributeDescriptions;
	}
};

// Object Uniform Buffer Object
struct ObjectUniformBufferObject {
	glm::mat4 model;
};

// Camera Uniform Buffer Object
struct CameraUniformBufferObject {
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec3 position;
};

// Lights Uniform Buffer Object
struct LightingUniformBufferObject {
	glm::vec4 numLights;
	glm::vec4 dirLightsDirection[MAX_DIR_LIGHTS];
	glm::vec4 dirLightsColor[MAX_DIR_LIGHTS];
	glm::vec4 pointLightsPosition[MAX_POINT_LIGHTS];
	glm::vec4 pointLightsColor[MAX_POINT_LIGHTS];
	glm::vec4 spotLightsPosition[MAX_SPOT_LIGHTS];
	glm::vec4 spotLightsDirection[MAX_SPOT_LIGHTS];
	glm::vec4 spotLightsColor[MAX_SPOT_LIGHTS];
	glm::vec4 spotLightsCutoffs[MAX_SPOT_LIGHTS];
};

// Shadow Uniform Buffer Object
struct ShadowUniformBufferObject {
	glm::vec4 numLights;
	glm::mat4 dirLightSpaces[MAX_DIR_LIGHTS];
	glm::mat4 spotLightSpaces[MAX_SPOT_LIGHTS];
};