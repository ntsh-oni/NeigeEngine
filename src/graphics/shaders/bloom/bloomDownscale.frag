#version 460

layout(set = 0, binding = 0) uniform sampler2D thresholdSampler;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Threshold {
	float threshold;
} tH;

void main() {
	vec2 texelSize = 1.0 / vec2(textureSize(thresholdSampler, 0));
	
	vec3 color = texture(thresholdSampler, uv + (vec2(-1, -1) * texelSize)).rgb;
	color += texture(thresholdSampler, uv + (vec2(1, -1) * texelSize)).rgb;
	color += texture(thresholdSampler, uv + (vec2(-1, 1) * texelSize)).rgb;
	color += texture(thresholdSampler, uv + (vec2(1, 1) * texelSize)).rgb;
	color /= 4.0;
	
	color = min(color, 10.0);
	color = max(color - tH.threshold, 0);
	outColor = vec4(color, 1.0);
}