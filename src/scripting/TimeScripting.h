#pragma once
#include "../utils/NeigeDefines.h"
#include "ScriptingResources.h"

struct TimeScripting {
	static double deltaTime;

	static void init();
	
	// Lua functions
	static int getDeltaTime(lua_State* L);
};