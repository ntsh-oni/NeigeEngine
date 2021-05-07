#version 460

#define MAX_DIR_LIGHTS 10
#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS 10

layout(set = 0, binding = 0) uniform Object {
	mat4 model;
} object;

layout(set = 0, binding = 1) uniform Camera {
	mat4 view;
	mat4 projection;
	vec3 pos;
} camera;

layout(set = 0, binding = 2) uniform Shadow {
	vec3 numLights;
	mat4 dirLightSpaces[MAX_DIR_LIGHTS];
	mat4 spotLightSpaces[MAX_SPOT_LIGHTS];
} shadow;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 outUv;
layout(location = 1) out int outDrawIndex;

void main() {
	outUv = uv;
	outDrawIndex = gl_DrawID;
	
	gl_Position = camera.projection * camera.view * object.model * vec4(position, 1.0);
}