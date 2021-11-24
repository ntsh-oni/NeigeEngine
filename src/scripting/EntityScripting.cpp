#include "EntityScripting.h"

extern ECS ecs;

int32_t EntityScripting::currentEntity = -1;

void EntityScripting::init() {
	lua_register(L, "getEntityId", getEntityId);
	lua_register(L, "destroyEntity", destroyEntity);
	lua_register(L, "hasRenderableComponent", hasRenderableComponent);
	lua_register(L, "getRenderableComponentModelPath", getRenderableComponentModelPath);
	lua_register(L, "hasScriptComponent", hasScriptComponent);
	lua_register(L, "getScriptComponentScriptPath", getScriptComponentScriptPath);
	lua_register(L, "hasTransformComponent", hasTransformComponent);
	lua_register(L, "getTransformComponentPosition", getTransformComponentPosition);
	lua_register(L, "getTransformComponentRotation", getTransformComponentRotation);
	lua_register(L, "getTransformComponentScale", getTransformComponentScale);
	lua_register(L, "setTransformComponentPosition", setTransformComponentPosition);
	lua_register(L, "setTransformComponentRotation", setTransformComponentRotation);
	lua_register(L, "setTransformComponentScale", setTransformComponentScale);

	std::string entityScript = FileTools::readAscii("../src/scripting/scripts/entity.lua");
	luaL_dostring(L, entityScript.c_str());
}

int EntityScripting::getEntityId(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		lua_pushnumber(L, static_cast<int>(currentEntity));

		return 1;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getEntityId()\" takes no parameter.");
		return 0;
	}
}

int EntityScripting::destroyEntity(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isnumber(L, -1)) {
			int entity = static_cast<int>(lua_tonumber(L, 1));

			bool hasScript = false;
			bool hasRenderable = false;
			if (ecs.hasComponent<Script>(entity)) {
				auto& entityScript = ecs.getComponent<Script>(entity);

				entityScript.destroyed = true;
				if (!entityScript.justDestroyed) {
					entityScript.justDestroyed = true;
				}

				if (ecs.hasComponent<Renderable>(entity)) {
					entityScript.canBeDestroyedNow = false;
				}

				hasScript = true;
			}

			if (ecs.hasComponent<Renderable>(entity)) {
				auto& entityRenderable = ecs.getComponent<Renderable>(entity);

				entityRenderable.destroyed = true;

				hasRenderable = true;
			}
			
			if (!hasScript && !hasRenderable) {
				ecs.destroyEntity(entity);
			}

			return 0;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"destroyEntity(int entity)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"destroyEntity(int entity)\" takes 1 integer parameter.");
		return 0;
	}
}

int EntityScripting::hasRenderableComponent(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));
		if (lua_isnumber(L, -1)) {
			lua_pushboolean(L, ecs.hasComponent<Renderable>(entity));

			return 1;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"hasRenderableComponent(int entity)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"hasRenderableComponent(int entity)\" takes 1 integer parameter.");
		return 0;
	}
}

int EntityScripting::getRenderableComponentModelPath(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));
		if (lua_isnumber(L, -1)) {
			if (ecs.hasComponent<Renderable>(entity)) {
				const auto& entityRenderable = ecs.getComponent<Renderable>(entity);

				lua_pushstring(L, entityRenderable.component.modelPath.c_str());

				return 1;
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Renderable component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"getRenderableComponentModelPath(int entity)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getRenderableComponentModelPath(int entity)\" takes 1 integer parameter.");
		return 0;
	}
}

int EntityScripting::hasScriptComponent(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));
		if (lua_isnumber(L, -1)) {
			lua_pushboolean(L, ecs.hasComponent<Script>(entity));

			return 1;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"hasScriptComponent(int entity)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"hasScriptComponent(int entity)\" takes 1 integer parameter.");
		return 0;
	}
}

int EntityScripting::getScriptComponentScriptPath(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));
		if (lua_isnumber(L, -1)) {
			if (ecs.hasComponent<Script>(entity)) {
				const auto& entityScript = ecs.getComponent<Script>(entity);

				lua_pushstring(L, entityScript.component.scriptPath.c_str());

				return 1;
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Script component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"getScriptComponentScriptPath(int entity)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getScriptComponentScriptPath(int entity)\" takes 1 integer parameter.");
		return 0;
	}
}

int EntityScripting::hasTransformComponent(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));
		if (lua_isnumber(L, -1)) {
			lua_pushboolean(L, ecs.hasComponent<Transform>(entity));

			return 1;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"hasTransformComponent(int entity)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"hasTransformComponent(int entity)\" takes 1 integer parameter.");
		return 0;
	}
}

int EntityScripting::getTransformComponentPosition(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));
		if (lua_isnumber(L, -1)) {
			if (ecs.hasComponent<Transform>(entity)) {
				const auto& entityTransform = ecs.getComponent<Transform>(entity);

				lua_pushnumber(L, entityTransform.component.position.x);
				lua_pushnumber(L, entityTransform.component.position.y);
				lua_pushnumber(L, entityTransform.component.position.z);

				return 3;
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Transform component.");
				return 0;
			}
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
			if (ecs.hasComponent<Transform>(entity)) {
				const auto& entityTransform = ecs.getComponent<Transform>(entity);

				lua_pushnumber(L, entityTransform.component.rotation.x);
				lua_pushnumber(L, entityTransform.component.rotation.y);
				lua_pushnumber(L, entityTransform.component.rotation.z);

				return 3;
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Transform component.");
				return 0;
			}
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
			if (ecs.hasComponent<Transform>(entity)) {
				const auto& entityTransform = ecs.getComponent<Transform>(entity);

				lua_pushnumber(L, entityTransform.component.scale.x);
				lua_pushnumber(L, entityTransform.component.scale.y);
				lua_pushnumber(L, entityTransform.component.scale.z);

				return 3;
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Script component.");
				return 0;
			}
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
			if (ecs.hasComponent<Transform>(entity)) {
				auto& entityTransform = ecs.getComponent<Transform>(entity);

				float x = static_cast<float>(lua_tonumber(L, 2));
				float y = static_cast<float>(lua_tonumber(L, 3));
				float z = static_cast<float>(lua_tonumber(L, 4));

				if (lua_isnumber(L, -2) && lua_isnumber(L, -3) && lua_isnumber(L, -4)) {
					entityTransform.component.position = glm::vec3(x, y, z);

					return 0;
				}
				else {
					NEIGE_SCRIPT_ERROR("Function \"setTransformComponentPosition(int entity, vec3 newPosition)\" takes 1 integer and 1 vec 3 parameters.");
					return 0;
				}
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Script component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"setTransformComponentPosition(int entity, vec3 newPosition)\" takes 1 integer and 1 vec 3 parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setTransformComponentPosition(int entity, vec3 newPosition)\" takes 1 integer and 1 vec 3 parameters.");
		return 0;
	}
}

int EntityScripting::setTransformComponentRotation(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 4) {
		uint32_t entity = static_cast<uint32_t>(lua_tonumber(L, 1));

		if (lua_isnumber(L, -1)) {
			if (ecs.hasComponent<Transform>(entity)) {
				auto& entityTransform = ecs.getComponent<Transform>(entity);

				float x = static_cast<float>(lua_tonumber(L, 2));
				float y = static_cast<float>(lua_tonumber(L, 3));
				float z = static_cast<float>(lua_tonumber(L, 4));

				if (lua_isnumber(L, -2) && lua_isnumber(L, -3) && lua_isnumber(L, -4)) {
					entityTransform.component.rotation = glm::vec3(x, y, z);

					return 0;
				}
				else {
					NEIGE_SCRIPT_ERROR("Function \"setTransformComponentRotation(int entity, vec3 newRotation)\" takes 1 integer and 1 vec 3 parameters.");
					return 0;
				}
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Script component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"setTransformComponentRotation(int entity, vec3 newRotation)\" takes 1 integer and 1 vec 3 parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setTransformComponentRotation(int entity, vec3 newRotation)\" takes 1 integer and 1 vec 3 parameters.");
		return 0;
	}
}

int EntityScripting::setTransformComponentScale(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 4) {
		uint32_t entity = static_cast<uint32_t>(lua_tonumber(L, 1));

		if (lua_isnumber(L, -1)) {
			if (ecs.hasComponent<Transform>(entity)) {
				auto& entityTransform = ecs.getComponent<Transform>(entity);

				float x = static_cast<float>(lua_tonumber(L, 2));
				float y = static_cast<float>(lua_tonumber(L, 3));
				float z = static_cast<float>(lua_tonumber(L, 4));

				if (lua_isnumber(L, -2) && lua_isnumber(L, -3) && lua_isnumber(L, -4)) {
					entityTransform.component.scale = glm::vec3(x, y, z);

					return 0;
				}
				else {
					NEIGE_SCRIPT_ERROR("Function \"setTransformComponentScale(int entity, vec3 newScale)\" takes 1 integer and 1 vec 3 parameters.");
					return 0;
				}
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Script component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"setTransformComponentScale(int entity, vec3 newScale)\" takes 1 integer and 1 vec 3 parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setTransformComponentScale(int entity, vec3 newScale)\" takes 1 integer and 1 vec 3 parameters.");
		return 0;
	}
}