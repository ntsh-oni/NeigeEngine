#pragma once
#include "ecs/ECS.h"
#include "ecs/components/Camera.h"
#include "ecs/components/Light.h"
#include "ecs/components/Renderable.h"
#include "ecs/components/Rigidbody.h"
#include "ecs/components/Script.h"
#include "ecs/components/Transform.h"
#include "ecs/systems/CameraSystem.h"
#include "ecs/systems/Lighting.h"
#include "graphics/Renderer.h"
#include "physics/Physics.h"
#include "scripting/Scripting.h"
#include "window/WindowResources.h"
#include <string>

struct Scene {
	std::string skyboxType = "NONE";
	std::string envmapPath = "";
};

struct GameInfo {
	std::string name = "";
	uint32_t windowWidth = 300;
	uint32_t windowHeight = 300;
	bool enableBloom = false;
	int bloomDownscale = 4;
	float bloomThreshold = 1.5f;
	int bloomBlurSize = 9;
	bool enableSSAO = false;
	int ssaoDownscale = 3;
	bool enableFXAA = false;
};

struct Game {
	std::shared_ptr<Renderer> renderer;
	std::shared_ptr<Physics> physics;
	std::shared_ptr<Scripting> scripting;
	double lastFrame = 0.0;
	GameInfo info = {};

	void init();
	void launch();
};
