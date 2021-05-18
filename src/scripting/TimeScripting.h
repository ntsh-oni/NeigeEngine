#pragma once
#include "../utils/NeigeDefines.h"
#include "ScriptingResources.h"
#include <ctime>

struct TimeScripting {
	static double deltaTime;

	static void init();
	
	// Lua functions
	static int getDeltaTime(lua_State* L);
};