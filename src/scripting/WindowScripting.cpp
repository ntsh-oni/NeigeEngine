#include "WindowScripting.h"

void WindowScripting::init() {
	lua_register(L, "isWindowFullscreen", isWindowFullscreen);
	lua_register(L, "showMouseCursor", showMouseCursor);
	lua_register(L, "getMousePosition", getMousePosition);
	lua_register(L, "setMousePosition", setMousePosition);
	lua_register(L, "toggleFullscreen", toggleFullscreen);
	lua_register(L, "getWindowSize", getWindowSize);
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
		if (lua_isboolean(L, -1)) {
			window.showCursor(show);

			return 0;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"showMouseCursor(bool show)\" takes 1 boolean parameter.");
			return 0;
		}
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

int WindowScripting::getWindowSize(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		lua_pushnumber(L, window.extent.width);
		lua_pushnumber(L, window.extent.height);

		return 2;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getWindowSize()\" takes no parameter.");
		return 0;
	}
}

int WindowScripting::getMousePosition(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		lua_pushnumber(L, mouseInputs.getXPosition());
		lua_pushnumber(L, mouseInputs.getYPosition());

		return 2;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getMousePosition()\" takes no parameter.");
		return 0;
	}
}

int WindowScripting::setMousePosition(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 2) {
		double x = lua_tonumber(L, 1);
		double y = lua_tonumber(L, 2);

		window.setCursorPosition(x, y);

		return 0;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setMousePosition(double x, double y)\" takes 2 parameters.");
		return 0;
	}
}