#version 460

layout(set = 0, binding = 0) uniform sampler2D accumulationSampler;
layout(set = 0, binding = 1) uniform sampler2D revealageSampler;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	float revealage = texture(revealageSampler, uv).r;
	if (revealage == 1.0) {
		discard;
	}
	
	vec4 accumulation = texture(accumulationSampler, uv);

	vec3 average = accumulation.rgb / max(accumulation.a, 0.00001);
	
	outColor = vec4(average, 1.0 - revealage);
}