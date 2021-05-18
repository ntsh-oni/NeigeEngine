#pragma once
#include "../ecs/ECS.h"
#include "../ecs/components/Camera.h"
#include "../ecs/components/Light.h"
#include "../ecs/components/Renderable.h"
#include "../ecs/components/Rigidbody.h"
#include "../ecs/components/Script.h"
#include "../ecs/components/Transform.h"
#include "../utils/NeigeDefines.h"
#include "ScriptingResources.h"
#include <string>

struct EntityScripting {
	static int32_t currentEntity;

	static void init();

	// Lua functions
	static int getEntityID(lua_State* L);
	static int getTransformComponentPosition(lua_State* L);
	static int getTransformComponentRotation(lua_State* L);
	static int getTransformComponentScale(lua_State* L);
	static int setTransformComponentPosition(lua_State* L);
	static int setTransformComponentRotation(lua_State* L);
	static int setTransformComponentScale(lua_State* L);
};