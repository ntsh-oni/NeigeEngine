#include "EntityScripting.h"

extern ECS ecs;

int32_t EntityScripting::currentEntity = -1;

void EntityScripting::init() {
	lua_register(L, "getEntityID", getEntityID);
	lua_register(L, "getTransformComponentPosition", getTransformComponentPosition);
	lua_register(L, "getTransformComponentRotation", getTransformComponentRotation);
	lua_register(L, "getTransformComponentScale", getTransformComponentScale);
	lua_register(L, "setTransformComponentPosition", setTransformComponentPosition);
	lua_register(L, "setTransformComponentRotation", setTransformComponentRotation);
	lua_register(L, "setTransformComponentScale", setTransformComponentScale);
}

int EntityScripting::getEntityID(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		lua_pushnumber(L, static_cast<int>(currentEntity));

		return 1;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getEntityID()\" takes no parameter.");
		return 0;
	}
}

int EntityScripting::getTransformComponentPosition(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));
		if (lua_isnumber(L, -1)) {
			auto& entityTransform = ecs.getComponent<Transform>(entity);

			// Position
			lua_pushnumber(L, entityTransform.position.x);
			lua_pushnumber(L, entityTransform.position.y);
			lua_pushnumber(L, entityTransform.position.z);

			return 3;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"getTransformComponentPosition(int entity)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getTransformComponentPosition(int entity)\" takes 1 integer parameter.");
		return 0;
	}
}

int EntityScripting::getTransformComponentRotation(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));
		if (lua_isnumber(L, -1)) {
			auto& entityTransform = ecs.getComponent<Transform>(entity);

			// Position
			lua_pushnumber(L, entityTransform.rotation.x);
			lua_pushnumber(L, entityTransform.rotation.y);
			lua_pushnumber(L, entityTransform.rotation.z);

			return 3;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"getTransformComponentRotation(int entity)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getTransformComponentRotation(int entity)\" takes 1 integer parameter.");
		return 0;
	}
}

int EntityScripting::getTransformComponentScale(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));
		if (lua_isnumber(L, -1)) {
			auto& entityTransform = ecs.getComponent<Transform>(entity);

			// Position
			lua_pushnumber(L, entityTransform.scale.x);
			lua_pushnumber(L, entityTransform.scale.y);
			lua_pushnumber(L, entityTransform.scale.z);

			return 3;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"getTransformComponentScale(int entity)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getTransformComponentScale(int entity)\" takes 1 integer parameter.");
		return 0;
	}
}

int EntityScripting::setTransformComponentPosition(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 4) {
		uint32_t entity = static_cast<uint32_t>(lua_tonumber(L, 1));

		if (lua_isnumber(L, -1)) {
			auto& entityTransform = ecs.getComponent<Transform>(entity);

			float x = static_cast<float>(lua_tonumber(L, 2));
			float y = static_cast<float>(lua_tonumber(L, 3));
			float z = static_cast<float>(lua_tonumber(L, 4));

			if (lua_isnumber(L, -2) && lua_isnumber(L, -3) && lua_isnumber(L, -4)) {
				entityTransform.position = glm::vec3(x, y, z);

				return 0;
			}
			else {
				NEIGE_SCRIPT_ERROR("Function \"setTransformComponentPosition(int entity, float x, float y, float z)\" takes 1 integer and 3 float parameters.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"setTransformComponentPosition(int entity, float x, float y, float z)\" takes 1 integer and 3 float parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setTransformComponentPosition(int entity, float x, float y, float z)\" takes 1 integer and 3 float parameters.");
		return 0;
	}
}

int EntityScripting::setTransformComponentRotation(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 4) {
		uint32_t entity = static_cast<uint32_t>(lua_tonumber(L, 1));

		if (lua_isnumber(L, -1)) {
			auto& entityTransform = ecs.getComponent<Transform>(entity);

			float x = static_cast<float>(lua_tonumber(L, 2));
			float y = static_cast<float>(lua_tonumber(L, 3));
			float z = static_cast<float>(lua_tonumber(L, 4));

			if (lua_isnumber(L, -2) && lua_isnumber(L, -3) && lua_isnumber(L, -4)) {
				entityTransform.rotation = glm::vec3(x, y, z);

				return 0;
			}
			else {
				NEIGE_SCRIPT_ERROR("Function \"setTransformComponentRotation(int entity, float x, float y, float z)\" takes 1 integer and 3 float parameters.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"setTransformComponentRotation(int entity, float x, float y, float z)\" takes 1 integer and 3 float parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setTransformComponentRotation(int entity, float x, float y, float z)\" takes 1 integer and 3 float parameters.");
		return 0;
	}
}

int EntityScripting::setTransformComponentScale(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 4) {
		uint32_t entity = static_cast<uint32_t>(lua_tonumber(L, 1));

		if (lua_isnumber(L, -1)) {
			auto& entityTransform = ecs.getComponent<Transform>(entity);

			float x = static_cast<float>(lua_tonumber(L, 2));
			float y = static_cast<float>(lua_tonumber(L, 3));
			float z = static_cast<float>(lua_tonumber(L, 4));

			if (lua_isnumber(L, -2) && lua_isnumber(L, -3) && lua_isnumber(L, -4)) {
				entityTransform.scale = glm::vec3(x, y, z);

				return 0;
			}
			else {
				NEIGE_SCRIPT_ERROR("Function \"setTransformComponentScale(int entity, float x, float y, float z)\" takes 1 integer and 3 float parameters.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"setTransformComponentScale(int entity, float x, float y, float z)\" takes 1 integer and 3 float parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setTransformComponentScale(int entity, float x, float y, float z)\" takes 1 integer and 3 float parameters.");
		return 0;
	}
}