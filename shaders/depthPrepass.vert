#version 460

layout(set = 0, binding = 0) uniform Object {
	mat4 model;
} object;

layout(set = 0, binding = 1) uniform Camera {
	mat4 view;
	mat4 projection;
	vec3 pos;
} camera;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 outUv;

void main() {
	outUv = uv;
	gl_Position = camera.projection * camera.view * vec4(vec3(object.model * vec4(position, 1.0)), 1.0);
}