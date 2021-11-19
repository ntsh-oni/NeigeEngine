#pragma once
#include "../../external/openal-soft/include/AL/al.h"
#include "../../external/openal-soft/include/AL/alc.h"
#include "../ecs/ECS.h"
#include "resources/AudioResources.h"
#include <string>

struct Audio : public System {
	ALCdevice* device;
	ALCcontext* context;

	void init();
	void update();
	void destroy();
};