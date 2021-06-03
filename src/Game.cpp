#include "Game.h"

extern ECS ecs;

void Game::init() {
	// Register components
	ecs.registerComponent<Camera>();
	ecs.registerComponent<Light>();
	ecs.registerComponent<Renderable>();
	ecs.registerComponent<Rigidbody>();
	ecs.registerComponent<Script>();
	ecs.registerComponent<Transform>();

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
	renderer->bloomThreshold = info.bloomThreshold;
	renderer->bloomBlurSize = info.bloomBlurSize;
	renderer->enableSSAO = info.enableSSAO;
	renderer->ssaoDownscale = info.ssaoDownscale;
	renderer->enableFXAA = info.enableFXAA;

	// Lighting system
	renderer->lighting = ecs.registerSystem<Lighting>();
	ComponentMask lightingMask;
	lightingMask.set(ecs.getComponentId<Light>());
	ecs.setSystemComponents<Lighting>(lightingMask);

	// Camera system
	ComponentMask cameraMask;
	cameraMask.set(ecs.getComponentId<Camera>());

	renderer->cameraSystem = ecs.registerSystem<CameraSystem>();
	ecs.setSystemComponents<CameraSystem>(cameraMask);

	// Physics system
	physics = ecs.registerSystem<Physics>();
	ComponentMask physicsMask;
	physicsMask.set(ecs.getComponentId<Renderable>());
	physicsMask.set(ecs.getComponentId<Rigidbody>());
	physicsMask.set(ecs.getComponentId<Transform>());
	ecs.setSystemComponents<Physics>(physicsMask);

	// Scripting system
	scripting = ecs.registerSystem<Scripting>();
	ComponentMask scriptingMask;
	scriptingMask.set(ecs.getComponentId<Script>());
	scriptingMask.set(ecs.getComponentId<Transform>());
	ecs.setSystemComponents<Scripting>(scriptingMask);
}

void Game::launch() {
	int nbFrames = 0;
	double fpsTime = glfwGetTime();

	window.init(info.name);
	scripting->init();
	renderer->init(info.name);

	while (!window.windowGotClosed()) {
		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastFrame;

		nbFrames++;
		if (currentTime - fpsTime >= 1.0) {
			DebugScripting::frametime = 1000.0 / static_cast<double>(nbFrames);
			nbFrames = 0;
			fpsTime += 1.0;
		}

		window.pollEvents();

		scripting->update(deltaTime);

		physics->update(std::min<double>(deltaTime, (1.0 / 60.0)));

		renderer->update();

		lastFrame = currentTime;
	}

	renderer->destroy();
	window.destroy();
}