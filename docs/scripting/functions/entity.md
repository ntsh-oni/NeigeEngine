# Scripting Documentation - Entity
Functions associated with entities and their components.

## Renderable
Prefix: ``renderable:``

### Functions
- **renderable new(string modelPath, string vertexShaderPath, string fragmentShaderPath, string tesselationControlShaderPath, string tesselationEvaluationShaderPath, string geometryShaderPath)**: Constructor.

## Script
Prefix: ``script:``

### Functions
- **script new(string scriptPath)**: Constructor.

## Transform
Prefix: ``transform:``

### Functions
- **transform new(vec3 position, vec3 rotation, vec3 scale)**: Constructor.

## Entity
Prefix: ``entity:``

### Functions
- **number getId()**: Returns the entity calling this script's id.
- **number create()**: Creates a new entity and returns its id.
- **destroy(number entity):** Destroys the entity.
- **bool hasRenderableComponent(number entity)**: Returns **true** if this entity has a renderable component. Returns **false** if this entity does not have a renderable component.
- **renderable getRenderableComponent(number entity)**: Returns this entity's renderable component. This function already checks if the entity has a renderable component.
- **addRenderableComponent(number entity, renderable newRenderable)**: Adds a renderable component to an entity that does not already have one.
- **bool hasScriptComponent(number entity)**: Returns **true** if this entity has a script component. Returns **false** if this entity does not have a script component.
- **script getScriptComponent(number entity)**: Returns this entity's script component. This function already checks if the entity has a script component.
- **addScriptComponent(number entity, script newScript)**: Adds a script component to an entity that does not already have one.
- **bool hasTransformComponent(number entity)**: Returns **true** if this entity has a transform component. Returns **false** if this entity does not have a transform component.
- **transform getTransformComponent(number entity)**: Returns this entity's transform component. This function already checks if the entity has a transform component.
- **setTransformComponent(number entity, transform newTransform)**: Changes the entity's transform component.
- **addTransformComponent(number entity, transform newTransform)**: Adds a transform component to an entity that does not already have one.

[>> Scripting documentation index](../index.md)