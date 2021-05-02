#include "AABB.h"

std::array<glm::vec3, 8> AABB::corners() {
	std::array<glm::vec3, 8> corners;
	corners[0] = glm::vec3(min.x, min.y, min.z);
	corners[1] = glm::vec3(min.x, min.y, max.z);
	corners[2] = glm::vec3(max.x, min.y, max.z);
	corners[3] = glm::vec3(max.x, min.y, min.z);
	corners[4] = glm::vec3(min.x, max.y, min.z);
	corners[5] = glm::vec3(min.x, max.y, max.z);
	corners[6] = glm::vec3(max.x, max.y, max.z);
	corners[7] = glm::vec3(max.x, max.y, min.z);

	return corners;
}

bool AABB::collision(glm::vec3 point) {
	return ((point.x >= min.x && point.x <= max.x)
		&& (point.y >= min.y && point.y <= max.y)
		&& (point.z >= min.z && point.z <= max.z));
}

bool AABB::collision(AABB other) {
	return ((min.x <= other.max.x && max.x >= other.min.x)
		&& (min.y <= other.max.y && max.y >= other.min.y)
		&& (min.z <= other.max.z && max.z >= other.min.z));
}