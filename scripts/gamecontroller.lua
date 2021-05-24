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
	
	drawText("Test.", fontId, 1.0, 1.0, 0.0, 50.0, 50.0)
end

function destroy()
	print("Entity destroyed.")
end