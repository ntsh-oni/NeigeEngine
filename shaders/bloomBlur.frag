#version 450

layout(set = 0, binding = 0) uniform sampler2D thresholdSampler;

layout(push_constant) uniform HorizontalBlur {
	int horizontalBlur;
} horizontalBlur;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

const float weights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
	vec2 texelSize = 1.0 / vec2(textureSize(thresholdSampler, 0));
	
	vec3 result = texture(thresholdSampler, uv).rgb * weights[0];
	if (horizontalBlur.horizontalBlur == 1) {
		for (int i = 1; i < 5; i++) {
			result += texture(thresholdSampler, uv + vec2(texelSize.x * i, 0.0)).rgb * weights[i];
			result += texture(thresholdSampler, uv - vec2(texelSize.x * i, 0.0)).rgb * weights[i];
		}
	}
	else {
		for (int i = 1; i < 5; i++) {
			result += texture(thresholdSampler, uv + vec2(0.0, texelSize.y * i)).rgb * weights[i];
			result += texture(thresholdSampler, uv - vec2(0.0, texelSize.y * i)).rgb * weights[i];
		}
	}

	outColor = vec4(result, 1.0);
}