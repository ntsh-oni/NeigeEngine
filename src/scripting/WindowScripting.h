#pragma once
#include "../utils/NeigeDefines.h"
#include "ScriptingResources.h"
#include "../window/WindowResources.h"

struct WindowScripting {
	static void init();
	
	// Lua functions
	static int isWindowFullscreen(lua_State* L);
	static int showMouseCursor(lua_State* L);
	static int toggleFullscreen(lua_State* L);
};