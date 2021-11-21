#include "TimeScripting.h"

double TimeScripting::deltaTime = 0.0;

void TimeScripting::init() {
	lua_register(L, "getDeltaTime", getDeltaTime);

	std::string timeScript = FileTools::readAscii("../src/scripting/scripts/time.lua");
	luaL_dostring(L, timeScript.c_str());
}

int TimeScripting::getDeltaTime(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		lua_pushnumber(L, deltaTime);

		return 1;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getDeltaTime()\" takes no parameter.");
		return 0;
	}
}