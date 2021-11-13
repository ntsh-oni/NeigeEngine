#pragma once
#include "../../../external/glm/glm/glm.hpp"

struct RigidbodyComponent {
	bool affectedByGravity;
};

struct Rigidbody {
	RigidbodyComponent component;

	glm::vec3 forces = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
};