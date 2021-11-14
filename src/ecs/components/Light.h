#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "../../../external/glm/glm/glm.hpp"
#include "../../graphics/structs/ShaderStructs.h"

struct LightComponent {
	LightType type;
	glm::vec3 color;
	glm::vec2 cutoffs;
};

struct Light {
	LightComponent component;

	int shadowMapIndex = 0;
	int savedShadowMapIndex = 0;
};