#pragma once
#include "../utils/resources/FileTools.h"
#include "../utils/NeigeDefines.h"
#include "resources/ScriptingResources.h"
#include <ctime>

struct TimeScripting {
	static double deltaTime;

	static void init();
	
	// Lua functions
	static int getDeltaTime(lua_State* L);
};