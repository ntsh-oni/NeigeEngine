#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../src/graphics/shaders/postprocess/tonemappingFunctions.glsl"

layout(set = 0, binding = 0) uniform sampler2D sceneSampler;
layout(set = 0, binding = 1) uniform sampler2D ssaoBlurredSampler;
layout(set = 0, binding = 2) uniform sampler2D bloomSampler;

layout(push_constant) uniform PushConstants {
	bool enableSSAO;
	bool enableBloom;
} pC;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 color = texture(sceneSampler, uv).rgb;
	
	if (pC.enableSSAO) {
		float ssaoBlurred = texture(ssaoBlurredSampler, uv).r;
		color *= ssaoBlurred;
	}
	
	if (pC.enableBloom) {
		vec3 bloom = textureLod(bloomSampler, uv, 1.5).rgb;
		bloom += textureLod(bloomSampler, uv, 3.5).rgb;
		bloom += textureLod(bloomSampler, uv, 4.5).rgb;
		bloom /= 3.0;
		color += bloom;
	}
	
	color = reinhard(color);
	outColor = vec4(color, 1.0);
}