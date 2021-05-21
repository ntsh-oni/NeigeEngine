#include "Scripting.h"
#include "../ecs/components/Script.h"
#include "../ecs/components/Transform.h"

extern ECS ecs;

void Scripting::init() {
	L = luaL_newstate();
	luaL_openlibs(L);

	CameraScripting::init();
	EntityScripting::init();
	InputScripting::init();
	TimeScripting::init();
	VectorScripting::init();
	WindowScripting::init();
}

void Scripting::update(double deltaTime) {
	TimeScripting::deltaTime = deltaTime;

	std::set<Entity> uninitializedEntities;
	std::set_difference(entities.begin(), entities.end(), initializedEntities.begin(), initializedEntities.end(), std::inserter(uninitializedEntities, uninitializedEntities.begin()));
	for (Entity entityToInit : uninitializedEntities) {
		auto& entityScript = ecs.getComponent<Script>(entityToInit);

		if (FileTools::exists(entityScript.scriptPath)) {
			EntityScripting::currentEntity = entityToInit;

			entityScript.script = FileTools::readAscii(entityScript.scriptPath);

			int r = luaL_dostring(L, entityScript.script.c_str());
			if (r != LUA_OK) {
				std::string error = lua_tostring(L, -1);
				NEIGE_SCRIPT_ERROR("\"" + entityScript.scriptPath + "\": load error: " + error);
				lua_pop(L, 1);
			}
			else {
				lua_getglobal(L, "init");
				if (!lua_isnil(L, -1)) {
					r = lua_pcall(L, 0, 0, 0);
					if (r != LUA_OK) {
						std::string error = lua_tostring(L, -1);
						NEIGE_SCRIPT_ERROR("\"" + entityScript.scriptPath + "\": call \"init\" function error: " + error);
						lua_pop(L, 1);
					}
				}
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Script file \"" + entityScript.scriptPath + "\" does not exist (entity " + std::to_string(entityToInit) + ").");
		}

		initializedEntities.emplace(entityToInit);
	}

	for (Entity entity : entities) {
		auto& entityScript = ecs.getComponent<Script>(entity);

		if (FileTools::exists(entityScript.scriptPath)) {
			EntityScripting::currentEntity = entity;

			int r = luaL_dostring(L, entityScript.script.c_str());
			if (r != LUA_OK) {
				std::string error = lua_tostring(L, -1);
				NEIGE_SCRIPT_ERROR("\"" + entityScript.scriptPath + "\": load error: " + error);
				lua_pop(L, 1);
			}
			else {
				lua_getglobal(L, "update");
				if (!lua_isnil(L, -1)) {
					r = lua_pcall(L, 0, 0, 0);
					if (r != LUA_OK) {
						std::string error = lua_tostring(L, -1);
						NEIGE_SCRIPT_ERROR("\"" + entityScript.scriptPath + "\": call \"update\" function error: " + error);
						lua_pop(L, 1);
					}
				}
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Script file \"" + entityScript.scriptPath + "\" does not exist (entity " + std::to_string(entity) + ").");
		}
	}
}

void Scripting::destroy() {
	if (L) {
		lua_close(L);
	}
}