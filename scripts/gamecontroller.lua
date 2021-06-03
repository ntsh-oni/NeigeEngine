function init()
	fontId = createFont("../modelfiles/JetBrainsMono-Regular.ttf", 24.0)
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
		print(showGraphicsMemoryChunks())
	end
	
	local ft = getFrametime()
	drawText("Frametime: " .. string.format("%.3f", ft), fontId, 1.0, 0.0, 0.0, 30.0, 30.0)
	drawText("FPS: " .. string.format("%.3f", 1000.0 / ft), fontId, 0.0, 1.0, 0.0, 30.0, 50.0)
end

function destroy()
	print("Entity destroyed.")
end