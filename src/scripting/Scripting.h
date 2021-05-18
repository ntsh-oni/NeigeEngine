#pragma once
#include "../ecs/ECS.h"
#include "../utils/resources/FileTools.h"
#include "EntityScripting.h"
#include "InputScripting.h"
#include "TimeScripting.h"
#include "VectorScripting.h"
#include "WindowScripting.h"
#include "ScriptingResources.h"

struct Scripting : public System {
	std::queue<Entity> uninitializedEntities;
	std::set<Entity> initializedEntities;

	void init();
	void update(double deltaTime);
	void destroy();
};