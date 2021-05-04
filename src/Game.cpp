#include "Game.h"

extern ECS ecs;

void Game::init() {
	// Register components
	ecs.registerComponent<Transform>();
	ecs.registerComponent<Camera>();
	ecs.registerComponent<Renderable>();
	ecs.registerComponent<Light>();
	ecs.registerComponent<Rigidbody>();

	// Register systems
	// Renderer system
	renderer = ecs.registerSystem<Renderer>();
	ComponentMask rendererMask;
	rendererMask.set(ecs.getComponentId<Renderable>());
	rendererMask.set(ecs.getComponentId<Transform>());
	ecs.setSystemComponents<Renderer>(rendererMask);

	// Graphics settings
	renderer->enableBloom = info.enableBloom;
	renderer->bloomDownscale = info.bloomDownscale;
	renderer->bloomBlurSize = info.bloomBlurSize;
	renderer->enableSSAO = info.enableSSAO;
	renderer->ssaoDownscale = info.ssaoDownscale;
	renderer->enableFXAA = info.enableFXAA;

	// Lighting system
	lighting = ecs.registerSystem<Lighting>();
	ComponentMask lightingMask;
	lightingMask.set(ecs.getComponentId<Light>());
	ecs.setSystemComponents<Lighting>(lightingMask);

	// Camera systems
	ComponentMask cameraMask;
	cameraMask.set(ecs.getComponentId<Camera>());

	cameraSystem = ecs.registerSystem<CameraSystem>();
	ecs.setSystemComponents<CameraSystem>(cameraMask);

	cameraControls = ecs.registerSystem<CameraControls>();
	ecs.setSystemComponents<CameraControls>(cameraMask);

	// Player system
	playerControls = ecs.registerSystem<PlayerControls>();
	ComponentMask playerControlsMask;
	playerControlsMask.set(ecs.getComponentId<Camera>());
	playerControlsMask.set(ecs.getComponentId<Rigidbody>());
	playerControlsMask.set(ecs.getComponentId<Transform>());
	ecs.setSystemComponents<PlayerControls>(playerControlsMask);

	// Physics systems
	physics = ecs.registerSystem<Physics>();
	ComponentMask physicsMask;
	physicsMask.set(ecs.getComponentId<Renderable>());
	physicsMask.set(ecs.getComponentId<Rigidbody>());
	physicsMask.set(ecs.getComponentId<Transform>());
	ecs.setSystemComponents<Physics>(physicsMask);
}

void Game::launch() {
	window.init(info.name);
	cameraControls->init();
	lighting->init();
	renderer->init(info.name);

	while (!window.windowGotClosed()) {
		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastFrame;

		window.pollEvents();

		cameraControls->update(deltaTime);

		lighting->update();

		physics->update(deltaTime > (1.0 / 60.0) ? (1.0 / 60.0) : deltaTime);

		renderer->update();

		lastFrame = currentTime;
	}

	renderer->destroy();
	window.destroy();
}