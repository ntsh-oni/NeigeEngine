#version 460

layout(set = 0, binding = 0) uniform sampler2D sceneSampler;
layout(set = 0, binding = 1) uniform sampler2D bloomSampler;
layout(set = 0, binding = 2) uniform sampler2D ssaoBlurredSampler;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 scene = texture(sceneSampler, uv).rgb;
	
	vec3 bloom = textureLod(bloomSampler, uv, 1.5).rgb;
	bloom += textureLod(bloomSampler, uv, 3.5).rgb;
	bloom += textureLod(bloomSampler, uv, 4.5).rgb;
	bloom /= 3.0;
	
	float ssaoBlurred = texture(ssaoBlurredSampler, uv).r;
	
	vec3 tmpColor = (scene * ssaoBlurred) + bloom;
	
	tmpColor = tmpColor / (tmpColor + vec3(1.0));
	outColor = vec4(tmpColor, 1.0);
}