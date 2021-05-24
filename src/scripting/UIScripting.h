#pragma once
#include "../utils/NeigeDefines.h"
#include "ScriptingResources.h"
#include "../window/WindowResources.h"
#include <string>

struct UIScripting {
	static void init();
	
	// Lua functions
	static int createFont(lua_State* L);
	static int drawText(lua_State* L);
};