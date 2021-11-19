#pragma once
#include "../utils/NeigeDefines.h"
#include "resources/ScriptingResources.h"
#include "../window/WindowResources.h"

struct WindowScripting {
	static void init();
	
	// Lua functions
	static int isWindowFullscreen(lua_State* L);
	static int setWindowFullscreen(lua_State* L);
	static int showMouseCursor(lua_State* L);
	static int getMousePosition(lua_State* L);
	static int setMousePosition(lua_State* L);
	static int getWindowSize(lua_State* L);
	static int setWindowSize(lua_State* L);
	static int setWindowTitle(lua_State* L);
	static int closeWindow(lua_State* L);
};