function update()
	if getKeyState("escape") == 0 then
		closeWindow()
	end
	
	if getMouseButtonState("left") == 0 then
		local cameraCount = getCameraCount()
		setMainCameraIndex((getMainCameraIndex() + 1) % cameraCount)
	end
end