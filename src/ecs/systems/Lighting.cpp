#include "Lighting.h"
#include "../../graphics/resources/ShaderResources.h"

extern ECS ecs;

void Lighting::init() {
	lights = &entities;
}