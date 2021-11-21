#pragma once
#include "../inputs/Inputs.h"
#include "../utils/resources/FileTools.h"
#include "../utils/NeigeDefines.h"
#include "resources/ScriptingResources.h"
#include <string>

struct InputScripting {
	static void init();

	// Lua functions
	static int getKeyState(lua_State* L);
	static int getMouseButtonState(lua_State* L);
};