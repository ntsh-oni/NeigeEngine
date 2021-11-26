renderable = {};

function renderable:new(modelPath, vertexShaderPath, fragmentShaderPath, tesselationControlShaderPath, tesselationEvaluationShaderPath, geometryShaderPath)
	return setmetatable({modelPath=modelPath or ' ', vertexShaderPath=vertexShaderPath or ' ', fragmentShaderPath=fragmentShaderPath or ' ', tesselationControlShaderPath=tesselationControlShaderPath or ' ', tesselationEvaluationShaderPath=tesselationEvaluationShaderPath or ' ', geometryShaderPath=geometryShaderPath or ' '}, getmetatable(self));
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

function entity:create()
	return createEntity();
end

function entity:destroy(entity)
	destroyEntity(entity);
end

function entity:hasRenderableComponent(entity)
	return hasRenderableComponent(entity);
end

function entity:getRenderableComponent(entity)
	local modelPath = getRenderableComponentModelPath(entity);
	local vertexShaderPath = getRenderableComponentVertexShaderPath(entity);
	local fragmentShaderPath = getRenderableComponentFragmentShaderPath(entity);
	local tesselationControlShaderPath = getRenderableComponentTesselationControlShaderPath(entity);
	local tesselationEvaluationShaderPath = getRenderableComponentTesselationEvaluationShaderPath(entity);
	local geometryShaderPath = getRenderableComponentGeometryShaderPath(entity);
	return renderable:new(modelPath, vertexShaderPath, fragmentShaderPath, tesselationControlShaderPath, tesselationEvaluationShaderPath, geometryShaderPath);
end

function entity:addRenderableComponent(entity, newRenderable)
	addRenderableComponent(entity, newRenderable.modelPath, newRenderable.vertexShaderPath, newRenderable.fragmentShaderPath, newRenderable.tesselationControlShaderPath, newRenderable.tesselationEvaluationShaderPath, newRenderable.geometryShaderPath);
end

function entity:hasScriptComponent(entity)
	return hasScriptComponent(entity);
end

function entity:getScriptComponent(entity)
	local scriptPath = getScriptComponentScriptPath(entity);
	return script:new(scriptPath);
end

function entity:addScriptComponent(entity, newScript)
	addScriptComponent(entity, newScript.scriptPath);
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

function entity:addTransformComponent(entity, newTransform)
	addTransformComponent(entity, newTransform.position.x, newTransform.position.y, newTransform.position.z, newTransform.rotation.x, newTransform.rotation.y, newTransform.rotation.z, newTransform.scale.x, newTransform.scale.y, newTransform.scale.z);
end