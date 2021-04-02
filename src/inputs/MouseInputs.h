#pragma once

enum struct ButtonState {
	PRESSED,
	HELD,
	RELEASED,
	NONE
};

struct MouseInputs {
private:
	double x;
	double y;
public:
	ButtonState leftButton = ButtonState::NONE;
	ButtonState rightButton = ButtonState::NONE;
	ButtonState middleButton = ButtonState::NONE;
	ButtonState fourButton = ButtonState::NONE;
	ButtonState fiveButton = ButtonState::NONE;
	ButtonState sixButton = ButtonState::NONE;
	ButtonState sevenButton = ButtonState::NONE;
	ButtonState eightButton = ButtonState::NONE;

	void update();
	double getXPosition();
	double getYPosition();
	void setPosition(double newX, double newY);
};