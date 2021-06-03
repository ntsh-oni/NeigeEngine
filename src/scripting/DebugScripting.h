#pragma once
#include "../graphics/resources/RendererResources.h"
#include "../graphics/resources/ShaderResources.h"
#include "../graphics/resources/UIResources.h"
#include "../utils/NeigeDefines.h"
#include "ScriptingResources.h"
#include <string>

struct DebugScripting {
	static double frametime;

	static void init();

	// Lua functions
	static int getFrametime(lua_State* L);
	static int getModelsList(lua_State* L);
	static int getTexturesList(lua_State* L);
	static int showGraphicsMemoryChunks(lua_State* L);
};