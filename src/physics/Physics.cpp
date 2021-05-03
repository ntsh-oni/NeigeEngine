#include "Physics.h"
#include "../ecs/components/Transform.h"
#include "../ecs/components/Renderable.h"
#include "../ecs/components/Rigidbody.h"

extern ECS ecs;

void Physics::update(double deltaTime) {
	for (const auto& entity : entities) {
		auto& entityRenderable = ecs.getComponent<Renderable>(entity);
		auto& entityRigidbody = ecs.getComponent<Rigidbody>(entity);
		auto& entityTransform = ecs.getComponent<Transform>(entity);

		entityRigidbody.acceleration = entityRigidbody.forces + (entityRigidbody.affectedByGravity ? gravity : glm::vec3(0.0f));

		glm::vec3 newPosition = (entityRigidbody.velocity * static_cast<float>(deltaTime));
		glm::vec3 newVelocity = (entityRigidbody.acceleration * static_cast<float>(deltaTime));

		// Collisions
		AABB modelAABB = models.at(entityRenderable.modelPath).aabb;
		AABB transformedAABBX = transformAABB(modelAABB, entityTransform.position + glm::vec3(newPosition.x, 0.0f, 0.0f), entityTransform.rotation, entityTransform.scale);
		AABB transformedAABBY = transformAABB(modelAABB, entityTransform.position + glm::vec3(0.0f, newPosition.y, 0.0f), entityTransform.rotation, entityTransform.scale);
		AABB transformedAABBZ = transformAABB(modelAABB, entityTransform.position + glm::vec3(0.0f, 0.0f, newPosition.z), entityTransform.rotation, entityTransform.scale);

		for (const auto& otherEntity : entities) {
			if (otherEntity != entity) {
				auto& otherEntityRenderable = ecs.getComponent<Renderable>(otherEntity);
				auto& otherEntityTransform = ecs.getComponent<Transform>(otherEntity);

				AABB otherModelAABB = models.at(otherEntityRenderable.modelPath).aabb;
				AABB otherTransformedAABB = transformAABB(otherModelAABB, otherEntityTransform.position, otherEntityTransform.rotation, otherEntityTransform.scale);

				// Collision on x-axis
				if (transformedAABBX.collision(otherTransformedAABB)) {
					newPosition.x = 0.0f;
					newVelocity.x = 0.0f;
				}

				// Collision on y-axis
				if (transformedAABBY.collision(otherTransformedAABB)) {
					newPosition.y = 0.0f;
					newVelocity.y = 0.0f;
				}

				// Collision on z-axis
				if (transformedAABBZ.collision(otherTransformedAABB)) {
					newPosition.z = 0.0f;
					newVelocity.z = 0.0f;
				}
			}
		}

		entityTransform.position += newPosition;
		entityRigidbody.velocity += newVelocity;
	}
}

AABB Physics::transformAABB(AABB aabb, glm::vec3 position, glm::vec3 rotation, glm::vec3 scaleRatio) {
	std::array<glm::vec3, 8> corners = aabb.corners();
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
		glm::vec3 cornerTransform = glm::vec3(transform * glm::vec4(corners[i], 1.0f));
		
		cornersX.push_back(cornerTransform.x);
		cornersY.push_back(cornerTransform.y);
		cornersZ.push_back(cornerTransform.z);
	}
	glm::vec3 minCorner = glm::vec3(*std::min_element(cornersX.begin(), cornersX.end()), *std::min_element(cornersY.begin(), cornersY.end()), *std::min_element(cornersZ.begin(), cornersZ.end()));
	glm::vec3 maxCorner = glm::vec3(*std::max_element(cornersX.begin(), cornersX.end()), *std::max_element(cornersY.begin(), cornersY.end()), *std::max_element(cornersZ.begin(), cornersZ.end()));
	return { minCorner, maxCorner };
}