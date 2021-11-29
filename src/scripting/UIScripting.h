#pragma once
#include "../utils/NeigeDefines.h"
#include "resources/ScriptingResources.h"
#include "../window/WindowResources.h"
#include <string>

struct UIScripting {
	static void init();
	
	// Lua functions
	static int loadFont(lua_State* L);
	static int loadSprite(lua_State* L);
	static int drawSprite(lua_State* L);
	static int drawText(lua_State* L);
};