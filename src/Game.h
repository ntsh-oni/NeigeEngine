#pragma once
#include "ecs/ECS.h"
#include "ecs/components/Transform.h"
#include "ecs/components/Camera.h"
#include "ecs/components/Light.h"
#include "ecs/components/Renderable.h"
#include "ecs/components/Rigidbody.h"
#include "ecs/systems/Lighting.h"
#include "ecs/systems/CameraSystem.h"
#include "ecs/systems/CameraControls.h"
#include "graphics/Renderer.h"
#include "physics/Physics.h"
#include "window/WindowResources.h"
#include <string>

struct Game {
	std::shared_ptr<Renderer> renderer;
	std::shared_ptr<Lighting> lighting;
	std::shared_ptr<CameraSystem> cameraSystem;
	std::shared_ptr<CameraControls> cameraControls;
	std::shared_ptr<Physics> physics;
	double lastFrame = 0.0;
	std::string applicationName = "";
	bool enableBloom;
	bool enableSSAO;
	bool enableFXAA;

	void init();
	void launch();
};
