#include "EntityScripting.h"

extern ECS ecs;

int32_t EntityScripting::currentEntity = -1;

void EntityScripting::init() {
	lua_register(L, "getEntityId", getEntityId);
	lua_register(L, "createEntity", createEntity);
	lua_register(L, "destroyEntity", destroyEntity);
	lua_register(L, "hasRenderableComponent", hasRenderableComponent);
	lua_register(L, "getRenderableComponentModelPath", getRenderableComponentModelPath);
	lua_register(L, "getRenderableComponentVertexShaderPath", getRenderableComponentVertexShaderPath);
	lua_register(L, "getRenderableComponentFragmentShaderPath", getRenderableComponentFragmentShaderPath);
	lua_register(L, "getRenderableComponentTesselationControlShaderPath", getRenderableComponentTesselationControlShaderPath);
	lua_register(L, "getRenderableComponentTesselationEvaluationShaderPath", getRenderableComponentTesselationEvaluationShaderPath);
	lua_register(L, "getRenderableComponentGeometryShaderPath", getRenderableComponentGeometryShaderPath);
	lua_register(L, "addRenderableComponent", addRenderableComponent);
	lua_register(L, "hasScriptComponent", hasScriptComponent);
	lua_register(L, "getScriptComponentScriptPath", getScriptComponentScriptPath);
	lua_register(L, "addScriptComponent", addScriptComponent);
	lua_register(L, "hasTransformComponent", hasTransformComponent);
	lua_register(L, "getTransformComponentPosition", getTransformComponentPosition);
	lua_register(L, "getTransformComponentRotation", getTransformComponentRotation);
	lua_register(L, "getTransformComponentScale", getTransformComponentScale);
	lua_register(L, "setTransformComponentPosition", setTransformComponentPosition);
	lua_register(L, "setTransformComponentRotation", setTransformComponentRotation);
	lua_register(L, "setTransformComponentScale", setTransformComponentScale);
	lua_register(L, "addTransformComponent", addTransformComponent);

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

int EntityScripting::createEntity(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		Entity entity = ecs.createEntity();

		lua_pushnumber(L, static_cast<int>(entity));

		return 1;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"createEntity()\" takes no parameter.");
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
		if (lua_isnumber(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

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
		if (lua_isnumber(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

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

int EntityScripting::getRenderableComponentVertexShaderPath(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isnumber(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

			if (ecs.hasComponent<Renderable>(entity)) {
				const auto& entityRenderable = ecs.getComponent<Renderable>(entity);

				lua_pushstring(L, entityRenderable.component.vertexShaderPath.c_str());

				return 1;
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Renderable component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"getRenderableComponentVertexShaderPath(int entity)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getRenderableComponentVertexShaderPath(int entity)\" takes 1 integer parameter.");
		return 0;
	}
}

int EntityScripting::getRenderableComponentFragmentShaderPath(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isnumber(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

			if (ecs.hasComponent<Renderable>(entity)) {
				const auto& entityRenderable = ecs.getComponent<Renderable>(entity);

				lua_pushstring(L, entityRenderable.component.fragmentShaderPath.c_str());

				return 1;
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Renderable component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"getRenderableComponentFragmentShaderPath(int entity)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getRenderableComponentFragmentShaderPath(int entity)\" takes 1 integer parameter.");
		return 0;
	}
}

int EntityScripting::getRenderableComponentTesselationControlShaderPath(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isnumber(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

			if (ecs.hasComponent<Renderable>(entity)) {
				const auto& entityRenderable = ecs.getComponent<Renderable>(entity);

				lua_pushstring(L, entityRenderable.component.tesselationControlShaderPath.c_str());

				return 1;
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Renderable component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"getRenderableComponentTesselationControlShaderPath(int entity)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getRenderableComponentTesselationControlShaderPath(int entity)\" takes 1 integer parameter.");
		return 0;
	}
}

int EntityScripting::getRenderableComponentTesselationEvaluationShaderPath(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isnumber(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

			if (ecs.hasComponent<Renderable>(entity)) {
				const auto& entityRenderable = ecs.getComponent<Renderable>(entity);

				lua_pushstring(L, entityRenderable.component.tesselationEvaluationShaderPath.c_str());

				return 1;
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Renderable component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"getRenderableComponentTesselationEvaluationShaderPath(int entity)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getRenderableComponentTesselationEvaluationShaderPath(int entity)\" takes 1 integer parameter.");
		return 0;
	}
}

int EntityScripting::getRenderableComponentGeometryShaderPath(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isnumber(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

			if (ecs.hasComponent<Renderable>(entity)) {
				const auto& entityRenderable = ecs.getComponent<Renderable>(entity);

				lua_pushstring(L, entityRenderable.component.geometryShaderPath.c_str());

				return 1;
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Renderable component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"getRenderableComponentGeometryShaderPath(int entity)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getRenderableComponentModelPath(int entity)\" takes 1 integer parameter.");
		return 0;
	}
}

int EntityScripting::addRenderableComponent(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 7) {
		if (lua_isnumber(L, -7), lua_isstring(L, -6), lua_isstring(L, -5), lua_isstring(L, -4), lua_isstring(L, -3), lua_isstring(L, -2), lua_isstring(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

			if (!ecs.hasComponent<Renderable>(entity)) {
				std::string modelPath = lua_tostring(L, 2);
				std::string vertexShaderPath = lua_tostring(L, 3);
				std::string fragmentShaderPath = lua_tostring(L, 4);
				std::string tesselationControlShaderPath = lua_tostring(L, 5);
				std::string tesselationEvaluationShaderPath = lua_tostring(L, 6);
				std::string geometryShaderPath = lua_tostring(L, 7);
				
				Renderable renderable;
				renderable.component.modelPath = modelPath;
				renderable.component.vertexShaderPath = vertexShaderPath;
				renderable.component.fragmentShaderPath = fragmentShaderPath;
				renderable.component.tesselationControlShaderPath = tesselationControlShaderPath;
				renderable.component.tesselationEvaluationShaderPath = tesselationEvaluationShaderPath;
				renderable.component.geometryShaderPath = geometryShaderPath;
				
				ecs.addComponent<Renderable>(entity, renderable);

				return 0;
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " already has a Renderable component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"addRenderableComponent(int entity, string modelPath, string vertexShaderPath, string fragmentShaderPath, string tesselationControlShaderPath, string tesselationEvaluationShaderPath, string geometryShaderPath)\" takes 1 integer and 6 strings parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"addRenderableComponent(int entity, string modelPath, string vertexShaderPath, string fragmentShaderPath, string tesselationControlShaderPath, string tesselationEvaluationShaderPath, string geometryShaderPath)\" takes 1 integer and 6 strings parameters.");
		return 0;
	}
}

int EntityScripting::hasScriptComponent(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isnumber(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

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
		if (lua_isnumber(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

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

int EntityScripting::addScriptComponent(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 2) {
		if (lua_isnumber(L, -2), lua_isstring(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

			if (!ecs.hasComponent<Script>(entity)) {
				std::string scriptPath = lua_tostring(L, 2);

				Script script;
				script.component.scriptPath = scriptPath;

				ecs.addComponent<Script>(entity, script);

				return 0;
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " already has a Script component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"addScriptComponent(int entity, string scriptPath)\" takes 1 integer and 1 string parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"addScriptComponent(int entity, string scriptPath)\" takes 1 integer and 1 string parameters.");
		return 0;
	}
}

int EntityScripting::hasTransformComponent(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isnumber(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

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
		if (lua_isnumber(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

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
		if (lua_isnumber(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

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
		if (lua_isnumber(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

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
		if (lua_isnumber(L, -1)) {
			uint32_t entity = static_cast<uint32_t>(lua_tonumber(L, 1));

			if (ecs.hasComponent<Transform>(entity)) {
				auto& entityTransform = ecs.getComponent<Transform>(entity);

				if (lua_isnumber(L, -2) && lua_isnumber(L, -3) && lua_isnumber(L, -4)) {
					float x = static_cast<float>(lua_tonumber(L, 2));
					float y = static_cast<float>(lua_tonumber(L, 3));
					float z = static_cast<float>(lua_tonumber(L, 4));

					entityTransform.component.position = glm::vec3(x, y, z);

					return 0;
				}
				else {
					NEIGE_SCRIPT_ERROR("Function \"setTransformComponentPosition(int entity, float newPositionX, float newPositionY, float newPositionZ)\" takes 1 integer and 3 floats parameters.");
					return 0;
				}
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Script component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"setTransformComponentPosition(int entity, float newPositionX, float newPositionY, float newPositionZ)\" takes 1 integer and 3 floats parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setTransformComponentPosition(int entity, float newPositionX, float newPositionY, float newPositionZ)\" takes 1 integer and 3 floats parameters.");
		return 0;
	}
}

int EntityScripting::setTransformComponentRotation(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 4) {
		if (lua_isnumber(L, -1)) {
			uint32_t entity = static_cast<uint32_t>(lua_tonumber(L, 1));

			if (ecs.hasComponent<Transform>(entity)) {
				auto& entityTransform = ecs.getComponent<Transform>(entity);

				if (lua_isnumber(L, -2) && lua_isnumber(L, -3) && lua_isnumber(L, -4)) {
					float x = static_cast<float>(lua_tonumber(L, 2));
					float y = static_cast<float>(lua_tonumber(L, 3));
					float z = static_cast<float>(lua_tonumber(L, 4));

					entityTransform.component.rotation = glm::vec3(x, y, z);

					return 0;
				}
				else {
					NEIGE_SCRIPT_ERROR("Function \"setTransformComponentRotation(int entity, float newRotationX, float newRotationY, float newRotationZ)\" takes 1 integer and 3 floats parameters.");
					return 0;
				}
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Script component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"setTransformComponentRotation(int entity, float newRotationX, float newRotationY, float newRotationZ)\" takes 1 integer and 3 floats parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setTransformComponentRotation(int entity, float newRotationX, float newRotationY, float newRotationZ)\" takes 1 integer and 3 floats parameters.");
		return 0;
	}
}

int EntityScripting::setTransformComponentScale(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 4) {
		if (lua_isnumber(L, -1)) {
			uint32_t entity = static_cast<uint32_t>(lua_tonumber(L, 1));

			if (ecs.hasComponent<Transform>(entity)) {
				auto& entityTransform = ecs.getComponent<Transform>(entity);

				if (lua_isnumber(L, -2) && lua_isnumber(L, -3) && lua_isnumber(L, -4)) {
					float x = static_cast<float>(lua_tonumber(L, 2));
					float y = static_cast<float>(lua_tonumber(L, 3));
					float z = static_cast<float>(lua_tonumber(L, 4));

					entityTransform.component.scale = glm::vec3(x, y, z);

					return 0;
				}
				else {
					NEIGE_SCRIPT_ERROR("Function \"setTransformComponentScale(int entity, float newScaleX, float newScaleY, float newScaleZ)\" takes 1 integer and 3 floats parameters.");
					return 0;
				}
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " does not have a Script component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"setTransformComponentScale(int entity, float newScaleX, float newScaleY, float newScaleZ)\" takes 1 integer and 3 floats parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setTransformComponentScale(int entity, float newScaleX, float newScaleY, float newScaleZ)\" takes 1 integer and 3 floats parameters.");
		return 0;
	}
}

int EntityScripting::addTransformComponent(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 10) {
		if (lua_isnumber(L, -10), lua_isnumber(L, -9), lua_isnumber(L, -8), lua_isnumber(L, -7), lua_isnumber(L, -6), lua_isnumber(L, -5), lua_isnumber(L, -4), lua_isnumber(L, -3), lua_isnumber(L, -2), lua_isnumber(L, -1)) {
			int32_t entity = static_cast<int32_t>(lua_tonumber(L, 1));

			if (!ecs.hasComponent<Transform>(entity)) {
				float positionX = static_cast<float>(lua_tonumber(L, 2));
				float positionY = static_cast<float>(lua_tonumber(L, 3));
				float positionZ = static_cast<float>(lua_tonumber(L, 4));
				float rotationX = static_cast<float>(lua_tonumber(L, 5));
				float rotationY = static_cast<float>(lua_tonumber(L, 6));
				float rotationZ = static_cast<float>(lua_tonumber(L, 7));
				float scaleX = static_cast<float>(lua_tonumber(L, 8));
				float scaleY = static_cast<float>(lua_tonumber(L, 9));
				float scaleZ = static_cast<float>(lua_tonumber(L, 10));

				Transform transform;
				transform.component.position = glm::vec3(positionX, positionY, positionZ);
				transform.component.rotation = glm::vec3(rotationX, rotationY, rotationZ);
				transform.component.scale = glm::vec3(scaleX, scaleY, scaleZ);

				ecs.addComponent<Transform>(entity, transform);

				return 0;
			}
			else {
				NEIGE_SCRIPT_ERROR("Entity " + std::to_string(entity) + " already has a Transform component.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"addTransformComponent(int entity, float positionX, float positionY, float positionZ, float rotationX, float rotationY, float rotationZ, float scaleX, float scaleY, float scaleZ)\" takes 1 integer and 9 floats parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"addTransformComponent(int entity, float positionX, float positionY, float positionZ, float rotationX, float rotationY, float rotationZ, float scaleX, float scaleY, float scaleZ)\" takes 1 integer and 9 floats parameters.");
		return 0;
	}
}