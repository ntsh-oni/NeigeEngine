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
end

function destroy()
	print("Entity destroyed")
end