#pragma once
#include "../../../external/glm/glm/glm.hpp"
#include "AABB.h"
#include <array>

struct Frustum {
	std::array<glm::vec4, 6> frustum;

	void calculateFrustum(const glm::mat4& view, const glm::mat4& projection);
	bool collision(const AABB& other);
};