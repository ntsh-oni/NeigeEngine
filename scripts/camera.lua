function init()
	showMouseCursor(false)
	mouseX, mouseY = getMousePosition()
	yaw = 0.0
	pitch = 0.0
	mouseView = true
	firstMove = true
end

function update()
	if getKeyState("f") == 0 then
		toggleFullscreen()
	end
	
	if getKeyState("r") == 0 then
		mouseView = not mouseView
		showMouseCursor(not mouseView)
	end
	
	if (firstMove) then
		mouseX, mouseY = getMousePosition()
		firstMove = false
	end

	local speed = 5.0
	local sensitivity = 1.5
	local dt = getDeltaTime()
	local id = getEntityID()
	local px, py, pz = getTransformComponentPosition(id)
	local rx, ry, rz = getTransformComponentRotation(id)
	
	if (mouseView) then
		local tmpMouseX, tmpMouseY = getMousePosition()
		local xOffset = (tmpMouseX - mouseX) * (sensitivity / 50.0)
		local yOffset = (tmpMouseY - mouseY) * (sensitivity / 50.0)
		mouseX = tmpMouseX
		mouseY = tmpMouseY
		yaw = (yaw + xOffset) % 360.0
		pitch = math.max(-89.0, math.min(89.0, pitch + yOffset))
	end
	
	rx = math.cos(math.rad(pitch)) * math.cos(math.rad(yaw))
	ry = -math.sin(math.rad(pitch))
	rz = math.cos(math.rad(pitch)) * math.sin(math.rad(yaw))
	rx, ry, rz = normalize(rx, ry, rz)
	
	setTransformComponentRotation(id, rx, ry, rz)

	if getKeyState("w") == 1 then
		px = px + (rx * speed * dt)
		py = py + (ry * speed * dt)
		pz = pz + (rz * speed * dt)
	end
	if getKeyState("a") == 1 then
		local tx, ty, tz = cross(rx, ry, rz, 0.0, 1.0, 0.0)
		tx, ty, tz = normalize(tx, ty, tz)
		px = px - (tx * speed * dt)
		py = py - (ty * speed * dt)
		pz = pz - (tz * speed * dt)
	end
	if getKeyState("s") == 1 then
		px = px - (rx * speed * dt)
		py = py - (ry * speed * dt)
		pz = pz - (rz * speed * dt)
	end
	if getKeyState("d") == 1 then
		local tx, ty, tz = cross(rx, ry, rz, 0.0, 1.0, 0.0)
		tx, ty, tz = normalize(tx, ty, tz)
		px = px + (tx * speed * dt)
		py = py + (ty * speed * dt)
		pz = pz + (tz * speed * dt)
	end
	if getKeyState("space") == 1 then
		py = py + (speed * dt)
	end
	if getKeyState("shift") == 1 then
		py = py - (speed * dt)
	end

	setTransformComponentPosition(id, px, py, pz)
end