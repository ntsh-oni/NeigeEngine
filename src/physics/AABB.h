#pragma once
#include "../../../external/glm/glm/glm.hpp"
#include "../../../external/glm/glm/gtc/matrix_transform.hpp"
#include <algorithm>
#include <array>
#include <vector>

struct AABB {
	glm::vec3 min;
	float padding1 = 0.0f;
	glm::vec3 max;
	float padding2 = 0.0f;

	std::array<glm::vec3, 8> corners();
	AABB transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scaleRatio);
	AABB transform(glm::mat4 transform);
	bool collision(const glm::vec3& point);
	bool collision(const AABB& other);
};