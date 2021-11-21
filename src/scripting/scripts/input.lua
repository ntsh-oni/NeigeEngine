InputState = {
	NONE = 0,
	PRESSED = 1,
	HELD = 2,
	RELEASED = 3,
}

input = {};

function input:getKeyState(key)
	return getKeyState(key);
end

function input:getMouseButtonState(button)
	return getMouseButtonState(button);
end