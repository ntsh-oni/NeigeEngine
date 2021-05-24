#version 460

layout(set = 0, binding = 0) uniform Camera {
	mat4 view;
	mat4 projection;
	vec3 pos;
} camera;

layout(push_constant) uniform QuadInfo {
	vec2 position[6];
	vec2 uv[6];
} qI;

layout(location = 0) out vec2 outUV;

void main() {
	outUV = qI.uv[gl_VertexIndex];
	gl_Position = camera.projection * vec4(qI.position[gl_VertexIndex], 0.0, 1.0);
}