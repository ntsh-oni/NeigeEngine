function init()
	fontId = createFont("../modelfiles/JetBrainsMono-Regular.ttf", 32.0)
end

function update()
	if getKeyState("escape") == 0 then
		closeWindow()
	end
	
	if getKeyState("l") == 0 then
		destroyEntity()
	end
	
	if getMouseButtonState("left") == 0 then
		local cameraCount = getCameraCount()
		setMainCameraIndex((getMainCameraIndex() + 1) % cameraCount)
	end
	
	if getMouseButtonState("right") == 0 then
		fontId = createFont("../modelfiles/JetBrainsMono-Regular.ttf", 16.0)
	end
	
	local mc = getMainCameraEntity()
	local rx, ry, rz = getTransformComponentRotation(mc)
	
	drawText(string.format("%.3f", rx) .. " " .. string.format("%.3f", ry) .. " " .. string.format("%.3f", rz), fontId, 1.0, 1.0, 0.0, 50.0, 50.0)
end

function destroy()
	print("Entity destroyed.")
end