#pragma once
#include "../ecs/ECS.h"
#include "../ecs/components/Camera.h"
#include "../ecs/components/Light.h"
#include "../ecs/components/Renderable.h"
#include "../ecs/components/Rigidbody.h"
#include "../ecs/components/Script.h"
#include "../ecs/components/Transform.h"
#include "../utils/NeigeDefines.h"
#include "resources/ScriptingResources.h"
#include <string>

struct EntityScripting {
	static int32_t currentEntity;

	static void init();

	// Lua functions
	static int getEntityId(lua_State* L);
	static int createEntity(lua_State* L);
	static int destroyEntity(lua_State* L);
	static int hasRenderableComponent(lua_State* L);
	static int getRenderableComponentModelPath(lua_State* L);
	static int getRenderableComponentVertexShaderPath(lua_State* L);
	static int getRenderableComponentFragmentShaderPath(lua_State* L);
	static int getRenderableComponentTesselationControlShaderPath(lua_State* L);
	static int getRenderableComponentTesselationEvaluationShaderPath(lua_State* L);
	static int getRenderableComponentGeometryShaderPath(lua_State* L);
	static int addRenderableComponent(lua_State* L);
	static int hasScriptComponent(lua_State* L);
	static int getScriptComponentScriptPath(lua_State* L);
	static int addScriptComponent(lua_State* L);
	static int hasTransformComponent(lua_State* L);
	static int getTransformComponentPosition(lua_State* L);
	static int getTransformComponentRotation(lua_State* L);
	static int getTransformComponentScale(lua_State* L);
	static int setTransformComponentPosition(lua_State* L);
	static int setTransformComponentRotation(lua_State* L);
	static int setTransformComponentScale(lua_State* L);
	static int addTransformComponent(lua_State* L);
};