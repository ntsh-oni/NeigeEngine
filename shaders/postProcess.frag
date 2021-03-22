#version 450

layout(set = 0, binding = 0) uniform sampler2D sceneSampler;
layout(set = 0, binding = 1) uniform sampler2D ssaoBlurredSampler;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 scene = texture(sceneSampler, uv).rgb;
	float ssaoBlurredSampler = texture(ssaoBlurredSampler, uv).r;
	
	scene = scene / (scene + vec3(1.0));
	outColor = vec4(scene * ssaoBlurredSampler, 1.0);
}