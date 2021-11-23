#include "PhysicsScripting.h"

extern ECS ecs;

void PhysicsScripting::init() {
	lua_register(L, "raycast", raycast);

	std::string physicsScript = FileTools::readAscii("../src/scripting/scripts/physics.lua");
	luaL_dostring(L, physicsScript.c_str());
}

int PhysicsScripting::raycast(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 6) {
		if (lua_isnumber(L, -6) && lua_isnumber(L, -5) && lua_isnumber(L, -4) && lua_isnumber(L, -3) && lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
			glm::vec3 rayOrigin = glm::vec3(static_cast<float>(lua_tonumber(L, 1)), static_cast<float>(lua_tonumber(L, 2)), static_cast<float>(lua_tonumber(L, 3)));
			glm::vec3 rayDirection = glm::vec3(static_cast<float>(lua_tonumber(L, 4)), static_cast<float>(lua_tonumber(L, 5)), static_cast<float>(lua_tonumber(L, 6)));

			glm::vec3 invDir = 1.0f / rayDirection;
			Entity closestEntity = -1;
			float t = std::numeric_limits<float>::max();

			for (Entity entity = 0; entity < MAX_ENTITIES; entity++) {
				if (ecs.hasComponent<Renderable>(entity)) {
					const auto& entityRenderable = ecs.getComponent<Renderable>(entity);
					const auto& entityTransform = ecs.getComponent<Transform>(entity);
					const AABB& aabb = entityRenderable.model->aabb.transform(entityTransform.component.position, entityTransform.component.rotation, entityTransform.component.scale);

					float t1 = (aabb.min.x - rayOrigin.x) * invDir.x;
					float t2 = (aabb.max.x - rayOrigin.x) * invDir.x;
					float t3 = (aabb.min.y - rayOrigin.y) * invDir.y;
					float t4 = (aabb.max.y - rayOrigin.y) * invDir.y;
					float t5 = (aabb.min.z - rayOrigin.z) * invDir.z;
					float t6 = (aabb.max.z - rayOrigin.z) * invDir.z;

					float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
					float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

					if (tmax >= 0.0 && tmin <= tmax && tmin < t) {
						closestEntity = entity;
						t = std::min(t, tmin);
					}
				}
			}

			lua_pushnumber(L, static_cast<int>(closestEntity));
			return 1;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"raycast(vec3 origin, vec3 direction)\" takes 2 vec3 parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"raycast(vec3 origin, vec3 direction)\" takes 2 vec3 parameters.");
		return 0;
	}
}