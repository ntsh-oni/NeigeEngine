ui = {};

function ui:loadSprite(spritePath)
	return loadSprite(spritePath);
end

function ui:loadFont(fontPath, height)
	return loadFont(fontPath, height);
end

function ui:drawSprite(spriteIndex, position, opacity)
	drawSprite(spriteIndex, position.x, position.y, opacity);
end

function ui:drawText(text, fontIndex, position, color)
	drawText(text, fontIndex, position.x, position.y, color.x, color.y, color.z, color.w);
end

function ui:drawRectangle(position, size, color)
	drawRectangle(position.x, position.y, size.x, size.y, color.x, color.y, color.z, color.w);
end

function ui:getSpriteSize(spriteIndex)
	local width, height = getSpriteSize(spriteIndex);
	return vec2:new(width, height);
end