ui = {};

function ui:loadSprite(spritePath)
	return loadSprite(spritePath);
end

function ui:loadFont(fontPath, height)
	return loadFont(fontPath, height);
end

function ui:drawSprite(spriteIndex, position)
	drawSprite(spriteIndex, position.x, position.y);
end

function ui:drawText(text, fontIndex, color, position)
	drawText(text, fontIndex, color.x, color.y, color.z, position.x, position.y);
end

function ui:getSpriteSize(spriteIndex)
	local width, height = getSpriteSize(spriteIndex);
	return vec2:new(width, height);
end