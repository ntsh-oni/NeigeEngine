window = {};

function window:isFullscreen()
	return isWindowFullscreen();
end

function window:setFullscreen(fullscreen)
	setWindowFullscreen(fullscreen);
end

function window:showMouseCursor(show)
	showMouseCursor(show);
end

function window:getMousePosition()
	local x, y = getMousePosition();
	return vec2:new(x, y);
end

function window:setMousePosition(newPosition)
	setMousePosition(newPosition.x, newPosition.y);
end

function window:getSize()
	local h, w = getWindowSize()
	return vec2:new(h, w);
end

function window:setSize(newSize)
	setWindowSize(newSize.x, newSize.y)
end

function window:setTitle(newTitle)
	setWindowTitle(newTitle);
end

function window:setIcon(newIconPath)
	setWindowIcon(newIconPath);
end

function window:close()
	closeWindow()
end