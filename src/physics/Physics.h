#pragma once
#include "../ecs/ECS.h"
#include "../../../external/glm/glm/glm.hpp"
#include "../../../external/glm/glm/gtc/matrix_transform.hpp"
#include "../../../external/glm/glm/gtx/string_cast.hpp"
#include "AABB.h"
#include <algorithm>
#include <vector>

struct Physics : public System {
	glm::vec3 gravity = glm::vec3(0.0f, -1.0f, 0.0);

	void update(double deltaTime);
	AABB transformAABB(AABB aabb, glm::vec3 position, glm::vec3 rotation, glm::vec3 scaleRatio);
};