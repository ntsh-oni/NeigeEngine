ui = {};

function ui:createFont(fontPath, height)
	return createFont(fontPath, height);
end

function ui:drawText(text, fontIndex, color, position)
	drawText(text, fontIndex, color.x, color.y, color.z, position.x, position.y);
end