#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "../../../external/glm/glm/glm.hpp"
#include "../../graphics/structs/ShaderStructs.h"

struct LightComponent {
	LightType type = LightType::DIRECTIONAL;
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec2 cutoffs = glm::vec2(0.0f, 0.0f);
};

struct Light {
	LightComponent component;

	int shadowMapIndex = 0;
	int savedShadowMapIndex = 0;
};