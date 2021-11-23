#pragma once
#include "../ecs/ECS.h"
#include "../utils/resources/FileTools.h"
#include "AudioScripting.h"
#include "CameraScripting.h"
#include "DebugScripting.h"
#include "EntityScripting.h"
#include "InputScripting.h"
#include "PhysicsScripting.h"
#include "TimeScripting.h"
#include "UIScripting.h"
#include "VectorScripting.h"
#include "WindowScripting.h"
#include "resources/ScriptingResources.h"

struct Scripting : public System {
	void init();
	void update(double deltaTime);
	void destroy();
};