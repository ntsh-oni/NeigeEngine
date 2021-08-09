function update()
	local speed = 5.0
	local dt = getDeltaTime()
	local id = getEntityID()
	local px, py, pz = getTransformComponentPosition(id)
	
	if getKeyState("up") == 1 then
		pz = pz - speed * dt
	end
	if getKeyState("down") == 1 then
		pz = pz + speed * dt
	end
	if getKeyState("left") == 1 then
		px = px - speed * dt
	end
	if getKeyState("right") == 1 then
		px = px + speed * dt
	end
	
	setTransformComponentPosition(id, px, py, pz)
end