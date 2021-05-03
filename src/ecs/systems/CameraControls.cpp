#include "CameraControls.h"
#include "../components/Camera.h"
#include "../components/Transform.h"
#include "../../inputs/Inputs.h"
#include "../../graphics/resources/ShaderResources.h"
#include "../../window/WindowResources.h"

extern ECS ecs;

void CameraControls::init() {
	window.showCursor(false);
}

void CameraControls::update(double deltaTime) {
	for (Entity entity : entities) {
		auto& cameraTransform = ecs.getComponent<Transform>(entity);

		// Camera rotation
		if (mouseView) {
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
		}
		else {
			if (keyboardInputs.leftKey == KeyState::HELD) {
				yaw -= static_cast<float>(sensitivity * deltaTime * 100.0f);
				yaw = glm::mod(yaw, 360.0f);
			}
			if (keyboardInputs.rightKey == KeyState::HELD) {
				yaw += static_cast<float>(sensitivity * deltaTime * 100.0f);
				yaw = glm::mod(yaw, 360.0f);
			}
			if (keyboardInputs.upKey == KeyState::HELD) {
				pitch -= static_cast<float>(sensitivity * deltaTime * 100.0f);
				pitch = std::clamp(pitch, -89.0f, 89.0f);
			}
			if (keyboardInputs.downKey == KeyState::HELD) {
				pitch += static_cast<float>(sensitivity * deltaTime * 100.0f);
				pitch = std::clamp(pitch, -89.0f, 89.0f);
			}
		}
		if (keyboardInputs.rKey == KeyState::PRESSED) {
			mouseView = !mouseView;
			if (mouseView) {
				window.showCursor(false);
				firstMove = true;
			}
			else {
				window.showCursor(true);
			}
		}
		cameraTransform.rotation.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		cameraTransform.rotation.y = -sin(glm::radians(pitch));
		cameraTransform.rotation.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		cameraTransform.rotation = glm::normalize(cameraTransform.rotation);

		// Camera position
		if (keyboardInputs.wKey == KeyState::HELD) {
			cameraTransform.position += cameraTransform.rotation * speed * static_cast<float>(deltaTime);
		}
		if (keyboardInputs.aKey == KeyState::HELD) {
			cameraTransform.position -= glm::normalize(glm::cross(cameraTransform.rotation, glm::vec3(0.0f, 1.0f, 0.0f))) * speed * static_cast<float>(deltaTime);
		}
		if (keyboardInputs.sKey == KeyState::HELD) {
			cameraTransform.position -= cameraTransform.rotation * speed * static_cast<float>(deltaTime);
		}
		if (keyboardInputs.dKey == KeyState::HELD) {
			cameraTransform.position += glm::normalize(glm::cross(cameraTransform.rotation, glm::vec3(0.0f, 1.0f, 0.0f))) * speed * static_cast<float>(deltaTime);
		}
		if (keyboardInputs.spaceKey == KeyState::HELD) {
			cameraTransform.position.y += speed * static_cast<float>(deltaTime);
		}
		if (keyboardInputs.shiftKey == KeyState::HELD) {
			cameraTransform.position.y -= speed * static_cast<float>(deltaTime);
		}
	}
}