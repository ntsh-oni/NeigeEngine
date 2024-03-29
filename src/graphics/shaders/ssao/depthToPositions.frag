#version 460

layout(set = 0, binding = 0) uniform sampler2D depthPrepass;

layout(set = 0, binding = 1) uniform Camera {
	mat4 view;
	mat4 projection;
	vec3 pos;
} camera;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	float depth = texture(depthPrepass, uv).x;
	
	vec4 clipSpace = vec4(uv * 2.0 - 1.0, depth, 1.0);
	vec4 viewSpace = inverse(camera.projection) * clipSpace;
	viewSpace.xyz /= viewSpace.w;
	
	outColor = vec4(vec3(viewSpace), 1.0);
}