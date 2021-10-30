#include "VectorScripting.h"

void VectorScripting::init() {
	std::string vec2Script = FileTools::readAscii("../src/scripting/scripts/vec2.lua");
	luaL_dostring(L, vec2Script.c_str());

	std::string vec3Script = FileTools::readAscii("../src/scripting/scripts/vec3.lua");
	luaL_dostring(L, vec3Script.c_str());

	std::string vec4Script = FileTools::readAscii("../src/scripting/scripts/vec4.lua");
	luaL_dostring(L, vec4Script.c_str());
}