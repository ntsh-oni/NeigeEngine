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
		AABB transformedAABBX = modelAABB.transform(entityTransform.position + glm::vec3(newPosition.x, 0.0f, 0.0f), entityTransform.rotation, entityTransform.scale);
		AABB transformedAABBY = modelAABB.transform(entityTransform.position + glm::vec3(0.0f, newPosition.y, 0.0f), entityTransform.rotation, entityTransform.scale);
		AABB transformedAABBZ = modelAABB.transform(entityTransform.position + glm::vec3(0.0f, 0.0f, newPosition.z), entityTransform.rotation, entityTransform.scale);

		for (const auto& otherEntity : entities) {
			if (otherEntity != entity) {
				auto& otherEntityRenderable = ecs.getComponent<Renderable>(otherEntity);
				auto& otherEntityTransform = ecs.getComponent<Transform>(otherEntity);

				AABB otherModelAABB = models.at(otherEntityRenderable.modelPath).aabb;
				AABB otherTransformedAABB = otherModelAABB.transform(otherEntityTransform.position, otherEntityTransform.rotation, otherEntityTransform.scale);

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