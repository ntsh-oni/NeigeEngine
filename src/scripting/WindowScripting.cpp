#include "WindowScripting.h"

void WindowScripting::init() {
	lua_register(L, "isWindowFullscreen", isWindowFullscreen);
	lua_register(L, "showMouseCursor", showMouseCursor);
	lua_register(L, "toggleFullscreen", toggleFullscreen);
}

int WindowScripting::isWindowFullscreen(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		lua_pushboolean(L, window.isFullscreen());

		return 1;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"isWindowFullscreen()\" takes no parameter.");
		return 0;
	}
}

int WindowScripting::showMouseCursor(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		bool show = lua_toboolean(L, 1);

		window.showCursor(show);

		return 0;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"showMouseCursor(bool show)\" takes 1 boolean parameter.");
		return 0;
	}
}

int WindowScripting::toggleFullscreen(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		window.toggleFullscreen();

		return 0;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"toggleFullscreen()\" takes no parameter.");
		return 0;
	}
}
