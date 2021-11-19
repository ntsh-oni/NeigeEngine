#pragma once
#include "../graphics/resources/ShaderResources.h"
#include "../utils/NeigeDefines.h"
#include "resources/ScriptingResources.h"
#include <string>

struct CameraScripting {
	static void init();

	// Lua functions
	static int getMainCameraIndex(lua_State* L);
	static int setMainCameraIndex(lua_State* L);
	static int getMainCameraEntity(lua_State* L);
	static int getCameraCount(lua_State* L);
};