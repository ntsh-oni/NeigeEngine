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
	renderer = ecs.registerSystem<Renderer>();
	ComponentMask rendererMask;
	rendererMask.set(ecs.getComponentId<Renderable>());
	rendererMask.set(ecs.getComponentId<Transform>());
	ecs.setSystemComponents<Renderer>(rendererMask);

	// Graphics settings
	renderer->enableBloom = info.enableBloom;
	renderer->bloomDownscale = info.bloomDownscale;
	renderer->blurSize = info.blurSize;
	renderer->enableSSAO = info.enableSSAO;
	renderer->ssaoDownscale = info.ssaoDownscale;
	renderer->enableFXAA = info.enableFXAA;

	lighting = ecs.registerSystem<Lighting>();
	ComponentMask lightingMask;
	lightingMask.set(ecs.getComponentId<Light>());
	ecs.setSystemComponents<Lighting>(lightingMask);

	ComponentMask cameraMask;
	cameraMask.set(ecs.getComponentId<Camera>());

	cameraSystem = ecs.registerSystem<CameraSystem>();
	ecs.setSystemComponents<CameraSystem>(cameraMask);

	cameraControls = ecs.registerSystem<CameraControls>();
	ecs.setSystemComponents<CameraControls>(cameraMask);

	physics = ecs.registerSystem<Physics>();
	ComponentMask physicsMask;
	physicsMask.set(ecs.getComponentId<Transform>());
	physicsMask.set(ecs.getComponentId<Rigidbody>());
	ecs.setSystemComponents<Physics>(physicsMask);
}

void Game::launch() {
	window.init(info.name);
	cameraControls->init();
	lighting->init();
	renderer->init(info.name);

	while (!window.windowGotClosed()) {
		window.pollEvents();

		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastFrame;

		cameraControls->update(deltaTime);

		lighting->update();

		physics->update(deltaTime);

		renderer->update();

		lastFrame = currentTime;
	}

	renderer->destroy();
	window.destroy();
}