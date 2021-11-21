#include "DebugScripting.h"

double DebugScripting::frametime = 0.0;

void DebugScripting::init() {
	lua_register(L, "getFrametime", getFrametime);
	lua_register(L, "getModelsList", getModelsList);
	lua_register(L, "getTexturesList", getTexturesList);
	lua_register(L, "showGraphicsMemoryChunks", showGraphicsMemoryChunks);

	std::string debugScript = FileTools::readAscii("../src/scripting/scripts/debug.lua");
	luaL_dostring(L, debugScript.c_str());
}

int DebugScripting::getFrametime(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		lua_pushnumber(L, frametime);

		return 1;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getFrametime()\" takes no parameter.");
		return 0;
	}
}

int DebugScripting::getModelsList(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		std::string list = "";
		for (std::unordered_map<std::string, Model>::iterator it = models.begin(); it != models.end(); it++) {
			list += it->first + "\n";
		}
		lua_pushstring(L, list.c_str());

		return 1;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getModelsList()\" takes no parameter.");
		return 0;
	}
}

int DebugScripting::getTexturesList(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		std::string list = "";
		for (Texture& texture : textures) {
			list += texture.key + "\n";
		}
		lua_pushstring(L, list.c_str());

		return 1;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getTexturesList()\" takes no parameter.");
		return 0;
	}
}

int DebugScripting::showGraphicsMemoryChunks(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		memoryAllocator.memoryAnalyzer();

		return 0;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"showGraphicsMemoryChunks()\" takes no parameter.");
		return 0;
	}
}
