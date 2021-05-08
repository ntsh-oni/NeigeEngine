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

AABB AABB::transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scaleRatio) {
	std::array<glm::vec3, 8> aabbCorners = corners();
	std::vector<float> cornersX;
	std::vector<float> cornersY;
	std::vector<float> cornersZ;

	glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), scaleRatio);
	glm::mat4 transform = translate * rotateX * rotateY * rotateZ * scale;

	for (size_t i = 0; i < 8; i++) {
		glm::vec3 cornerTransform = glm::vec3(transform * glm::vec4(aabbCorners[i], 1.0f));

		cornersX.push_back(cornerTransform.x);
		cornersY.push_back(cornerTransform.y);
		cornersZ.push_back(cornerTransform.z);
	}
	glm::vec3 minCorner = glm::vec3(*std::min_element(cornersX.begin(), cornersX.end()), *std::min_element(cornersY.begin(), cornersY.end()), *std::min_element(cornersZ.begin(), cornersZ.end()));
	glm::vec3 maxCorner = glm::vec3(*std::max_element(cornersX.begin(), cornersX.end()), *std::max_element(cornersY.begin(), cornersY.end()), *std::max_element(cornersZ.begin(), cornersZ.end()));
	return { minCorner, 0.0f, maxCorner, 0.0f };
}

AABB AABB::transform(glm::mat4 transform) {
	std::array<glm::vec3, 8> aabbCorners = corners();
	std::vector<float> cornersX;
	std::vector<float> cornersY;
	std::vector<float> cornersZ;

	for (size_t i = 0; i < 8; i++) {
		glm::vec3 cornerTransform = glm::vec3(transform * glm::vec4(aabbCorners[i], 1.0f));

		cornersX.push_back(cornerTransform.x);
		cornersY.push_back(cornerTransform.y);
		cornersZ.push_back(cornerTransform.z);
	}
	glm::vec3 minCorner = glm::vec3(*std::min_element(cornersX.begin(), cornersX.end()), *std::min_element(cornersY.begin(), cornersY.end()), *std::min_element(cornersZ.begin(), cornersZ.end()));
	glm::vec3 maxCorner = glm::vec3(*std::max_element(cornersX.begin(), cornersX.end()), *std::max_element(cornersY.begin(), cornersY.end()), *std::max_element(cornersZ.begin(), cornersZ.end()));
	return { minCorner, 0.0f, maxCorner, 0.0f };
}

bool AABB::collision(const glm::vec3& point) {
	return ((point.x >= min.x && point.x <= max.x)
		&& (point.y >= min.y && point.y <= max.y)
		&& (point.z >= min.z && point.z <= max.z));
}

bool AABB::collision(const AABB& other) {
	return ((min.x <= other.max.x && max.x >= other.min.x)
		&& (min.y <= other.max.y && max.y >= other.min.y)
		&& (min.z <= other.max.z && max.z >= other.min.z));
}