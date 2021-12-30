#pragma once
#include "../utils/NeigeDefines.h"
#include "../audio/resources/AudioResources.h"
#include "resources/ScriptingResources.h"
#include <string>

struct AudioScripting {
	static void init();

	// Lua functions
	static int loadAudio(lua_State* L);
	static int playSound(lua_State* L);
	static int stopSound(lua_State* L);
	static int pauseSound(lua_State* L);
	static int isPlaying(lua_State* L);
	static int setGain(lua_State* L);
	static int setPitch(lua_State* L);
	static int getSoundCount(lua_State* L);
};