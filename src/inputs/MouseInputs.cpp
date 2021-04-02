#include "MouseInputs.h"

void MouseInputs::update() {
	if (leftButton == ButtonState::PRESSED) {
		leftButton = ButtonState::HELD;
	} else if (leftButton == ButtonState::RELEASED) {
		leftButton = ButtonState::NONE;
	}

	if (rightButton == ButtonState::PRESSED) {
		rightButton = ButtonState::HELD;
	}
	else if (rightButton == ButtonState::RELEASED) {
		rightButton = ButtonState::NONE;
	}

	if (middleButton == ButtonState::PRESSED) {
		middleButton = ButtonState::HELD;
	}
	else if (middleButton == ButtonState::RELEASED) {
		middleButton = ButtonState::NONE;
	}

	if (fourButton == ButtonState::PRESSED) {
		fourButton = ButtonState::HELD;
	}
	else if (fourButton == ButtonState::RELEASED) {
		fourButton = ButtonState::NONE;
	}

	if (fiveButton == ButtonState::PRESSED) {
		fiveButton = ButtonState::HELD;
	}
	else if (fiveButton == ButtonState::RELEASED) {
		fiveButton = ButtonState::NONE;
	}

	if (sixButton == ButtonState::PRESSED) {
		sixButton = ButtonState::HELD;
	}
	else if (sixButton == ButtonState::RELEASED) {
		sixButton = ButtonState::NONE;
	}

	if (sevenButton == ButtonState::PRESSED) {
		sevenButton = ButtonState::HELD;
	}
	else if (sevenButton == ButtonState::RELEASED) {
		sevenButton = ButtonState::NONE;
	}

	if (eightButton == ButtonState::PRESSED) {
		eightButton = ButtonState::HELD;
	}
	else if (eightButton == ButtonState::RELEASED) {
		eightButton = ButtonState::NONE;
	}
}

double MouseInputs::getXPosition() {
	return x;
}

double MouseInputs::getYPosition() {
	return y;
}

void MouseInputs::setPosition(double newX, double newY) {
	x = newX;
	y = newY;
}
