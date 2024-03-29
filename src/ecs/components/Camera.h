#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "../../../external/glm/glm/glm.hpp"
#include "../../../external/glm/glm/gtc/matrix_transform.hpp"
#include "../../physics/Frustum.h"

struct CameraComponent {
	float FOV = 45.0f;
	float nearPlane = 0.3f;
	float farPlane = 200.0f;
};

struct Camera {
	CameraComponent component;

	glm::mat4 view;
	glm::mat4 projection;
	Frustum frustum;

	static glm::mat4 createLookAtView(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
		return glm::lookAt(eye, center, up);
	}

	static glm::mat4 createPerspectiveProjection(float FOV, float aspectRatio, float nearPlane, float farPlane, bool inverseY) {
		glm::mat4 projection = glm::perspective(glm::radians(FOV), aspectRatio, nearPlane, farPlane);
		projection[1][1] *= inverseY ? -1 : 1;

		return projection;
	}

	static glm::mat4 createOrthoProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
		return glm::ortho(left, right, bottom, top, nearPlane, farPlane);
	}

	static glm::mat4 createOrthoProjection(float left, float right, float bottom, float top) {
		return glm::ortho(left, right, bottom, top);
	}
};