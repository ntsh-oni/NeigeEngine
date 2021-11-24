#include "Scripting.h"
#include "../ecs/components/Script.h"
#include "../ecs/components/Transform.h"

extern ECS ecs;

void Scripting::init() {
	L = luaL_newstate();
	luaL_openlibs(L);

	VectorScripting::init();
	AudioScripting::init();
	CameraScripting::init();
	DebugScripting::init();
	EntityScripting::init();
	InputScripting::init();
	PhysicsScripting::init();
	TimeScripting::init();
	UIScripting::init();
	WindowScripting::init();
}

void Scripting::update(double deltaTime) {
	TimeScripting::deltaTime = deltaTime;

	// Init
	for (Entity entity : entities) {
		auto& entityScript = ecs.getComponent<Script>(entity);

		if (!entityScript.initialized && !entityScript.destroyed) {
			if (FileTools::exists(entityScript.component.scriptPath)) {
				EntityScripting::currentEntity = entity;

				entityScript.script = FileTools::readAscii(entityScript.component.scriptPath);

				int r = luaL_dostring(L, entityScript.script.c_str());
				if (r != LUA_OK) {
					std::string error = lua_tostring(L, -1);
					NEIGE_SCRIPT_ERROR("\"" + entityScript.component.scriptPath + "\": load error: " + error);
					lua_pop(L, 1);
				}
				else {
					lua_getglobal(L, "init");
					if (!lua_isnil(L, -1)) {
						r = lua_pcall(L, 0, 0, 0);
						if (r != LUA_OK) {
							std::string error = lua_tostring(L, -1);
							NEIGE_SCRIPT_ERROR("\"" + entityScript.component.scriptPath + "\": call \"init\" function error: " + error);
							lua_pop(L, 1);
						}
					}
				}
			}
			else {
				NEIGE_SCRIPT_ERROR("Script file \"" + entityScript.component.scriptPath + "\" does not exist (entity " + std::to_string(entity) + ").");
			}

			entityScript.initialized = true;
		}
	}

	// Update
	for (Entity entity : entities) {
		auto& entityScript = ecs.getComponent<Script>(entity);

		if (!entityScript.destroyed) {
			if (FileTools::exists(entityScript.component.scriptPath)) {
				EntityScripting::currentEntity = entity;

				int r = luaL_dostring(L, entityScript.script.c_str());
				if (r != LUA_OK) {
					std::string error = lua_tostring(L, -1);
					NEIGE_SCRIPT_ERROR("\"" + entityScript.component.scriptPath + "\": load error: " + error);
					lua_pop(L, 1);
				}
				else {
					lua_getglobal(L, "update");
					if (!lua_isnil(L, -1)) {
						r = lua_pcall(L, 0, 0, 0);
						if (r != LUA_OK) {
							std::string error = lua_tostring(L, -1);
							NEIGE_SCRIPT_ERROR("\"" + entityScript.component.scriptPath + "\": call \"update\" function error: " + error);
							lua_pop(L, 1);
						}
					}
				}
			}
			else {
				NEIGE_SCRIPT_ERROR("Script file \"" + entityScript.component.scriptPath + "\" does not exist (entity " + std::to_string(entity) + ").");
			}
		}
	}

	// Destroy
	std::vector<Entity> entitiesToDestroy;
	for (Entity entity : entities) {
		auto& entityScript = ecs.getComponent<Script>(entity);

		if (entityScript.destroyed && entityScript.justDestroyed) {
			if (FileTools::exists(entityScript.component.scriptPath)) {
				EntityScripting::currentEntity = entity;

				int r = luaL_dostring(L, entityScript.script.c_str());
				if (r != LUA_OK) {
					std::string error = lua_tostring(L, -1);
					NEIGE_SCRIPT_ERROR("\"" + entityScript.component.scriptPath + "\": load error: " + error);
					lua_pop(L, 1);
				}
				else {
					lua_getglobal(L, "destroy");
					if (!lua_isnil(L, -1)) {
						r = lua_pcall(L, 0, 0, 0);
						if (r != LUA_OK) {
							std::string error = lua_tostring(L, -1);
							NEIGE_SCRIPT_ERROR("\"" + entityScript.component.scriptPath + "\": call \"destroy\" function error: " + error);
							lua_pop(L, 1);
						}
					}
				}
			}
			else {
				NEIGE_SCRIPT_ERROR("Script file \"" + entityScript.component.scriptPath + "\" does not exist (entity " + std::to_string(entity) + ").");
			}

			entityScript.justDestroyed = false;

			if (entityScript.canBeDestroyedNow) {
				entitiesToDestroy.push_back(entity);
			}
		}
	}

	for (Entity entity : entitiesToDestroy) {
		ecs.destroyEntity(entity);
	}
}

void Scripting::destroy() {
	if (L) {
		lua_close(L);
	}
}