function init()
	window:showMouseCursor(false)
	local winSize = window:getSize()
	mouse = vec2:new(winSize.x / 2.0, winSize.y / 2.0)
	window:setMousePosition(mouse)
	yaw = 0.0
	pitch = 0.0
	mouseView = true
end

function update()
	local speed = 5.0
	local sensitivity = 1.5
	local dt = time:getDeltaTime()
	local id = entity:getId()
	local entityTransform = entity:getTransformComponent(id)
	
	if id ~= camera:getMainCameraEntity() then
		do return end
	end
	
	if input:getKeyState("f") == InputState.PRESSED then
		window:setFullscreen(not window:isFullscreen())
		if mouseView then
			local winSize = window:getSize()
			mouse.x = winSize.x / 2.0
			mouse.y = winSize.y / 2.0
			window:setMousePosition(mouse)
		end
	end
	
	if input:getKeyState("r") == InputState.PRESSED then
		mouseView = not mouseView
		window:showMouseCursor(not mouseView)
		if mouseView then
			local winSize = window:getSize()
			mouse.x = winSize.x / 2.0
			mouse.y = winSize.y / 2.0
			window:setMousePosition(mouse);
		end
	end
	
	if (mouseView) then
		local tmpmouse = window:getMousePosition()
		local xOffset = (tmpmouse.x - mouse.x) * (sensitivity / 50.0)
		local yOffset = (tmpmouse.y - mouse.y) * (sensitivity / 50.0)
		mouse = tmpmouse
		yaw = (yaw + xOffset) % 360.0
		pitch = math.max(-89.0, math.min(89.0, pitch + yOffset))
	end
	
	entityTransform.rotation.x = math.cos(math.rad(pitch)) * math.cos(math.rad(yaw))
	entityTransform.rotation.y = -math.sin(math.rad(pitch))
	entityTransform.rotation.z = math.cos(math.rad(pitch)) * math.sin(math.rad(yaw))
	entityTransform.rotation = vec3:normalize(entityTransform.rotation)

	if input:getKeyState("w") == InputState.HELD then
		entityTransform.position.x = entityTransform.position.x + (entityTransform.rotation.x * speed * dt)
		entityTransform.position.y = entityTransform.position.y + (entityTransform.rotation.y * speed * dt)
		entityTransform.position.z = entityTransform.position.z + (entityTransform.rotation.z * speed * dt)
	end
	if input:getKeyState("a") == InputState.HELD then
		local t = vec3:cross(entityTransform.rotation, vec3:new(0.0, 1.0, 0.0))
		t = vec3:normalize(t)
		entityTransform.position.x = entityTransform.position.x - (t.x * speed * dt)
		entityTransform.position.y = entityTransform.position.y - (t.y * speed * dt)
		entityTransform.position.z = entityTransform.position.z - (t.z * speed * dt)
	end
	if input:getKeyState("s") == InputState.HELD then
		entityTransform.position.x = entityTransform.position.x - (entityTransform.rotation.x * speed * dt)
		entityTransform.position.y = entityTransform.position.y - (entityTransform.rotation.y * speed * dt)
		entityTransform.position.z = entityTransform.position.z - (entityTransform.rotation.z * speed * dt)
	end
	if input:getKeyState("d") == InputState.HELD then
		local t = vec3:cross(entityTransform.rotation, vec3:new(0.0, 1.0, 0.0))
		t = vec3:normalize(t)
		entityTransform.position.x = entityTransform.position.x + (t.x * speed * dt)
		entityTransform.position.y = entityTransform.position.y + (t.y * speed * dt)
		entityTransform.position.z = entityTransform.position.z + (t.z * speed * dt)
	end
	if input:getKeyState("space") == InputState.HELD then
		entityTransform.position.y = entityTransform.position.y + (speed * dt)
	end
	if input:getKeyState("shift") == InputState.HELD then
		entityTransform.position.y = entityTransform.position.y - (speed * dt)
	end

	entity:setTransformComponent(id, entityTransform)
end