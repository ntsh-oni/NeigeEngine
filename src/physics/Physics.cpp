#include "Physics.h"
#include "../ecs/components/Transform.h"
#include "../ecs/components/Renderable.h"
#include "../ecs/components/Rigidbody.h"

extern ECS ecs;

void Physics::update(double deltaTime) {
	for (const auto& entity : entities) {
		auto& entityRigidbody = ecs.getComponent<Rigidbody>(entity);

		if (entityRigidbody.affectedByGravity) {
			auto& entityRenderable = ecs.getComponent<Renderable>(entity);
			auto& entityTransform = ecs.getComponent<Transform>(entity);

			glm::vec3 newPosition = (entityRigidbody.velocity * static_cast<float>(deltaTime));
			glm::vec3 newVelocity = (gravity * entityRigidbody.acceleration * static_cast<float>(deltaTime));

			// Collisions
			bool collide = false;
			AABB newAABB = { entityRenderable.aabb.min + newPosition, entityRenderable.aabb.max + newPosition };
			for (const auto& otherEntity : entities) {
				if (otherEntity != entity) {
					auto& otherEntityRenderable = ecs.getComponent<Renderable>(otherEntity);
					if (newAABB.collision(otherEntityRenderable.aabb)) {
						collide = true;
						break;
					}
				}
			}

			if (!collide) {
				entityTransform.position += newPosition;
				entityRigidbody.velocity += newVelocity;
			}
		}
	}
}
