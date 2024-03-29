#include "CameraSystem.h"
#include "../components/Camera.h"
#include "../../window/WindowResources.h"
#include "../../graphics/resources/ShaderResources.h"

extern ECS ecs;

void CameraSystem::init() {
	bool firstCamera = true;
	for (Entity camera : entities) {
		auto& cameraCamera = ecs.getComponent<Camera>(camera);
		cameraCamera.projection = Camera::createPerspectiveProjection(cameraCamera.component.FOV, window.extent.width / static_cast<float>(window.extent.height), cameraCamera.component.nearPlane, cameraCamera.component.farPlane, true);

		if (firstCamera) {
			mainCamera = camera;
			firstCamera = false;
		}
	}

	cameras = &entities;
}