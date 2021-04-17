#version 450

layout(set = 0, binding = 0) uniform sampler2D sceneSampler;
layout(set = 0, binding = 1) uniform sampler2D bloomSampler;
layout(set = 0, binding = 2) uniform sampler2D ssaoBlurredSampler;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	vec2 texelSize = 1.0 / vec2(textureSize(sceneSampler, 0));
	vec3 scene = texture(sceneSampler, uv).rgb;
	vec3 bloom = texture(bloomSampler, uv).rgb;
	float ssaoBlurred = texture(ssaoBlurredSampler, uv).r;
	
	vec3 tmpColor = scene + bloom;
	
	tmpColor = tmpColor / (tmpColor + vec3(1.0));
	tmpColor *= ssaoBlurred;
	outColor = vec4(tmpColor, 1.0);
}