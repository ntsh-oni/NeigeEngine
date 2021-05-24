#pragma once
#include "../ecs/ECS.h"
#include "../utils/resources/FileTools.h"
#include "CameraScripting.h"
#include "EntityScripting.h"
#include "InputScripting.h"
#include "TimeScripting.h"
#include "UIScripting.h"
#include "VectorScripting.h"
#include "WindowScripting.h"
#include "ScriptingResources.h"

struct Scripting : public System {
	void init();
	void update(double deltaTime);
	void destroy();
};