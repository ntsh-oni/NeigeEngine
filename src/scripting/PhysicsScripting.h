#pragma once
#include "../ecs/ECS.h"
#include "../ecs/components/Renderable.h"
#include "../ecs/components/Transform.h"
#include "../utils/resources/FileTools.h"
#include "../utils/NeigeDefines.h"
#include "resources/ScriptingResources.h"
#include <algorithm>
#include <limits>

struct PhysicsScripting {
	static void init();
	
	// Lua functions
	static int raycast(lua_State* L);
};