#version 460

layout(constant_id = 0) const bool enableSSAO = true;
layout(constant_id = 1) const bool enableBloom = true;

layout(set = 0, binding = 0) uniform sampler2D sceneSampler;
layout(set = 0, binding = 1) uniform sampler2D ssaoBlurredSampler;
layout(set = 0, binding = 2) uniform sampler2D bloomSampler;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 color = texture(sceneSampler, uv).rgb;
	
	if (enableSSAO) {
		float ssaoBlurred = texture(ssaoBlurredSampler, uv).r;
		color *= ssaoBlurred;
	}
	
	if (enableBloom) {
		vec3 bloom = textureLod(bloomSampler, uv, 1.5).rgb;
		bloom += textureLod(bloomSampler, uv, 3.5).rgb;
		bloom += textureLod(bloomSampler, uv, 4.5).rgb;
		bloom /= 3.0;
		color += bloom;
	}
	
	color = color / (color + vec3(1.0));
	outColor = vec4(color, 1.0);
}