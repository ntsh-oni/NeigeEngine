#pragma once

extern "C" {
#include "../../external/lua/lua.h"
#include "../../external/lua/lauxlib.h"
#include "../../external/lua/lualib.h"
}

inline lua_State* L = nullptr;