function init()
end

function update()
	local transformSpeed = 5.0
	local transformComponent = entity:getTransformComponent(entity:getId())
	if input:getKeyState("left") == InputState.HELD then
		transformComponent.position.z = transformComponent.position.z - (transformSpeed * time:getDeltaTime())
	end
	if input:getKeyState("right") == InputState.HELD then
		transformComponent.position.z = transformComponent.position.z + (transformSpeed * time:getDeltaTime())
	end
	if input:getKeyState("up") == InputState.HELD then
		transformComponent.position.x = transformComponent.position.x + (transformSpeed * time:getDeltaTime())
	end
	if input:getKeyState("down") == InputState.HELD then
		transformComponent.position.x = transformComponent.position.x - (transformSpeed * time:getDeltaTime())
	end

	entity:setTransformComponent(entity:getId(), transformComponent)
end

function destroy()
end