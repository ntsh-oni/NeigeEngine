#include "PlayerControls.h"
#include "../components/Camera.h"
#include "../components/Rigidbody.h"
#include "../components/Transform.h"
#include "../../inputs/Inputs.h"
#include "../../window/WindowResources.h"

extern ECS ecs;

void PlayerControls::init() {
	window.showCursor(false);
}

void PlayerControls::update(double deltaTime) {
	for (Entity entity : entities) {
		auto& playerCamera = ecs.getComponent<Camera>(entity);
		auto& playerRigidbody = ecs.getComponent<Rigidbody>(entity);
		auto& playerTransform = ecs.getComponent<Transform>(entity);

		if (firstMove) {
			x = mouseInputs.getXPosition();
			y = mouseInputs.getYPosition();
			firstMove = false;
		}

		double xOffset = (mouseInputs.getXPosition() - x) * (static_cast<double>(sensitivity) / 50.0f);
		double yOffset = (mouseInputs.getYPosition() - y) * (static_cast<double>(sensitivity) / 50.0f);
		x = mouseInputs.getXPosition();
		y = mouseInputs.getYPosition();
		yaw = glm::mod(yaw + static_cast<float>(xOffset), 360.0f);
		pitch = std::clamp(pitch + static_cast<float>(yOffset), -89.0f, 89.0f);

		playerTransform.rotation.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		playerTransform.rotation.y = -sin(glm::radians(pitch));
		playerTransform.rotation.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		playerTransform.rotation = glm::normalize(playerTransform.rotation);

		glm::vec3 horizontalRotation = glm::vec3(playerTransform.rotation.x, 0.0f, playerTransform.rotation.z);
		float actualSpeed = speed;
		if (keyboardInputs.shiftKey == KeyState::HELD) {
			actualSpeed *= 1.5;
		}
		if (keyboardInputs.wKey == KeyState::HELD) {
			playerTransform.position += horizontalRotation * actualSpeed * static_cast<float>(deltaTime);
		}
		if (keyboardInputs.aKey == KeyState::HELD) {
			playerTransform.position -= glm::normalize(glm::cross(horizontalRotation, glm::vec3(0.0f, 1.0f, 0.0f))) * actualSpeed * static_cast<float>(deltaTime);
		}
		if (keyboardInputs.sKey == KeyState::HELD) {
			playerTransform.position -= horizontalRotation * actualSpeed * static_cast<float>(deltaTime);
		}
		if (keyboardInputs.dKey == KeyState::HELD) {
			playerTransform.position += glm::normalize(glm::cross(horizontalRotation, glm::vec3(0.0f, 1.0f, 0.0f))) * actualSpeed * static_cast<float>(deltaTime);
		}
	}
}