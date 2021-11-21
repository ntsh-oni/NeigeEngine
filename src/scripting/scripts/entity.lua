renderable = {};

function renderable:new(modelPath)
	return setmetatable({modelPath=modelPath or ' '}, getmetatable(self));
end

script = {};

function script:new(scriptPath)
	return setmetatable({scriptPath=scriptPath or ' '}, getmetatable(self));
end

transform = {};

function transform:new(position, rotation, scale)
	return setmetatable({position=position or vec3:new(0.0, 0.0, 0.0), rotation=rotation or vec3:new(0.0, 0.0, 0.0), scale=scale or vec3:new(0.0, 0.0, 0.0)}, getmetatable(self));
end

entity = {};

function entity:getId()
	return getEntityId();
end

function entity:destroy(entity)
	destroyEntity(entity);
end

function entity:hasRenderableComponent(entity)
	return hasRenderableComponent(entity);
end

function entity:getRenderableComponent(entity)
	local modelPath = getRenderableComponentModelsPath(entity);
	return renderable:new(modelPath);
end

function entity:hasScriptComponent(entity)
	return hasScriptComponent(entity);
end

function entity:getScriptComponent(entity)
	local scriptPath = getScriptComponentScriptPath(entity);
	return script:new(scriptPath);
end

function entity:hasTransformComponent(entity)
	return hasTransformComponent(entity)
end

function entity:getTransformComponent(entity)
	local px, py, pz = getTransformComponentPosition(entity);
	local position = vec3:new(px, py, pz);
	local rx, ry, rz = getTransformComponentRotation(entity);
	local rotation = vec3:new(rx, ry, rz);
	local sx, sy, sz = getTransformComponentScale(entity);
	local scale = vec3:new(sx, sy, sz);
	return transform:new(position, rotation, scale);
end

function entity:setTransformComponent(entity, newTransform)
	setTransformComponentPosition(entity, newTransform.position.x, newTransform.position.y, newTransform.position.z);
	setTransformComponentRotation(entity, newTransform.rotation.x, newTransform.rotation.y, newTransform.rotation.z);
	setTransformComponentScale(entity, newTransform.scale.x, newTransform.scale.y, newTransform.scale.z);
end