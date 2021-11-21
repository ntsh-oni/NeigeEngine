#include "InputScripting.h"

void InputScripting::init() {
	lua_register(L, "getKeyState", getKeyState);
	lua_register(L, "getMouseButtonState", getMouseButtonState);

	std::string inputScript = FileTools::readAscii("../src/scripting/scripts/input.lua");
	luaL_dostring(L, inputScript.c_str());
}

int InputScripting::getKeyState(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isstring(L, -1)) {
			std::string key = lua_tostring(L, 1);

			if (key == "q") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.qKey));
			}
			else if (key == "w") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.wKey));
			}
			else if (key == "e") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.eKey));
			}
			else if (key == "r") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.rKey));
			}
			else if (key == "t") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.tKey));
			}
			else if (key == "y") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.yKey));
			}
			else if (key == "u") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.uKey));
			}
			else if (key == "i") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.iKey));
			}
			else if (key == "o") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.oKey));
			}
			else if (key == "p") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.pKey));
			}
			else if (key == "a") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.aKey));
			}
			else if (key == "s") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.sKey));
			}
			else if (key == "d") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.dKey));
			}
			else if (key == "f") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.fKey));
			}
			else if (key == "g") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.gKey));
			}
			else if (key == "h") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.hKey));
			}
			else if (key == "j") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.jKey));
			}
			else if (key == "k") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.kKey));
			}
			else if (key == "l") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.lKey));
			}
			else if (key == "z") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.zKey));
			}
			else if (key == "x") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.xKey));
			}
			else if (key == "c") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.cKey));
			}
			else if (key == "v") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.vKey));
			}
			else if (key == "b") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.bKey));
			}
			else if (key == "n") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.nKey));
			}
			else if (key == "m") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.mKey));
			}
			else if (key == "left") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.leftKey));
			}
			else if (key == "right") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.rightKey));
			}
			else if (key == "up") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.upKey));
			}
			else if (key == "down") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.downKey));
			}
			else if (key == "escape") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.escapeKey));
			}
			else if (key == "space") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.spaceKey));
			}
			else if (key == "shift") {
				lua_pushnumber(L, static_cast<int>(keyboardInputs.shiftKey));
			}
			else {
				NEIGE_SCRIPT_ERROR("Function \"getKeyState(string key)\": key \"" + key + "\" does not exist.");
				return 0;
			} 

			return 1;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"getKeyState(string key)\" takes 1 string parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getKeyState(string key)\" takes 1 string parameter.");
		return 0;
	}
}

int InputScripting::getMouseButtonState(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_tostring(L, -1)) {
			std::string button = lua_tostring(L, 1);

			if (button == "left") {
				lua_pushnumber(L, static_cast<int>(mouseInputs.leftButton));
			}
			else if (button == "right") {
				lua_pushnumber(L, static_cast<int>(mouseInputs.rightButton));
			}
			else if (button == "middle") {
				lua_pushnumber(L, static_cast<int>(mouseInputs.middleButton));
			}
			else if (button == "4") {
				lua_pushnumber(L, static_cast<int>(mouseInputs.fourButton));
			}
			else if (button == "5") {
				lua_pushnumber(L, static_cast<int>(mouseInputs.fiveButton));
			}
			else if (button == "6") {
				lua_pushnumber(L, static_cast<int>(mouseInputs.sixButton));
			}
			else if (button == "7") {
				lua_pushnumber(L, static_cast<int>(mouseInputs.sevenButton));
			}
			else if (button == "8") {
				lua_pushnumber(L, static_cast<int>(mouseInputs.eightButton));
			}
			else {
				NEIGE_SCRIPT_ERROR("Function \"getMouseButtonState(string button)\": button \"" + button + "\" does not exist.");
				return 0;
			}

			return 1;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"getMouseButtonState(string key)\" takes 1 string parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getMouseButtonState(string key)\" takes 1 string parameter.");
		return 0;
	}
}