#pragma once
#include "../../../external/glm/glm/glm.hpp"
#include "../../../external/glm/glm/gtx/string_cast.hpp"
#include "../ecs/ECS.h"
#include "AABB.h"
#include <vector>

struct Physics : public System {
	glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0);

	void update(double deltaTime);
};