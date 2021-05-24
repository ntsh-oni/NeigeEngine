#include "WindowScripting.h"

void WindowScripting::init() {
	lua_register(L, "isWindowFullscreen", isWindowFullscreen);
	lua_register(L, "setWindowFullscreen", setWindowFullscreen);
	lua_register(L, "showMouseCursor", showMouseCursor);
	lua_register(L, "getMousePosition", getMousePosition);
	lua_register(L, "setMousePosition", setMousePosition);
	lua_register(L, "getWindowSize", getWindowSize);
	lua_register(L, "setWindowSize", setWindowSize);
	lua_register(L, "setWindowTitle", setWindowTitle);
	lua_register(L, "closeWindow", closeWindow);
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

int WindowScripting::setWindowFullscreen(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isboolean(L, -1)) {
			bool fullscreen = lua_toboolean(L, 1);

			window.setFullscreen(fullscreen);

			return 0;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"setWindowFullscreen(bool fullscreen)\" takes 1 boolean parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setWindowFullscreen(bool fullscreen)\" takes 1 boolean parameter.");
		return 0;
	}
}

int WindowScripting::showMouseCursor(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isboolean(L, -1)) {
			bool show = lua_toboolean(L, 1);
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
		if (lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
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
	else {
		NEIGE_SCRIPT_ERROR("Function \"setMousePosition(double x, double y)\" takes 2 parameters.");
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

int WindowScripting::setWindowSize(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 2) {
		if (lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
			int width = static_cast<int>(lua_tonumber(L, 1));
			int height = static_cast<int>(lua_tonumber(L, 2));

			window.setWindowSize(width, height);

			return 0;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"setWindowSize(int width, int height)\" takes 2 integer parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setWindowSize(int width, int height)\" takes 2 integer parameters.");
		return 0;
	}
}

int WindowScripting::setWindowTitle(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isstring(L, -1)) {
			std::string title = lua_tostring(L, 1);

			window.setWindowTitle(title);

			return 0;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"setWindowTitle(string title)\" takes 1 string parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setWindowTitle(string title)\" takes 1 string parameter.");
		return 0;
	}
}

int WindowScripting::closeWindow(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		window.closeWindow();

		return 0;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"closeWindow()\" takes no parameter.");
		return 0;
	}
}
