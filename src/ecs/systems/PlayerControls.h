#pragma once
#include "../ECS.h"
#include <algorithm>

struct PlayerControls : public System {
	void init();
	void update(double deltaTime);

	float speed = 5.0f;
	float sensitivity = 1.5f;
	float yaw = 0.0f;
	float pitch = 0.0f;

	bool firstMove = true;
	double x;
	double y;
};
