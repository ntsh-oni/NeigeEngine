#pragma once
#include "../../../external/glm/glm/glm.hpp"
#include <array>

struct AABB {
	glm::vec3 min;
	glm::vec3 max;

	std::array<glm::vec3, 8> corners();
	bool collision(glm::vec3 point);
	bool collision(AABB other);
};