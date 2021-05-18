#pragma once
#define GLM_FORCE_RADIANS
#include "../../external/glm/glm/glm.hpp"
#include "../utils/NeigeDefines.h"
#include "ScriptingResources.h"

struct VectorScripting {
	static void init();
	
	// Lua functions
	static int normalize(lua_State* L);
	static int dot(lua_State* L);
	static int cross(lua_State* L);
};