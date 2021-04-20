#version 460

layout(set = 0, binding = 0) uniform sampler2D bloomSampler;

layout(push_constant) uniform HorizontalBlur {
	int horizontalBlur;
} horizontalBlur;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

const float weights[33] = float[] (
	0.004013,
	0.005554,
	0.007527,
	0.00999,
	0.012984,
	0.016524,
	0.020594,
	0.025133,
	0.030036,
	0.035151,
	0.040283,
	0.045207,
	0.049681,
	0.053463,
	0.056341,
	0.058141,
	0.058754,
	0.058141,
	0.056341,
	0.053463,
	0.049681,
	0.045207,
	0.040283,
	0.035151,
	0.030036,
	0.025133,
	0.020594,
	0.016524,
	0.012984,
	0.00999,
	0.007527,
	0.005554,
	0.004013
);
const float offsets[33] = float[] (
	-16,
	-15,
	-14,
	-13,
	-12,
	-11,
	-10,
	-9,
	-8,
	-7,
	-6,
	-5,
	-4,
	-3,
	-2,
	-1,
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	16
);

void main() {
	vec2 texelSize = 1.0 / vec2(textureSize(bloomSampler, 0));
	
	vec3 result = vec3(0.0);
	if (horizontalBlur.horizontalBlur == 1) {
		for (int i = 0; i < 33; i++) {
			result += texture(bloomSampler, uv + vec2(texelSize.x * offsets[i], 0.0)).rgb * weights[i];
		}
	}
	else {
		for (int i = 0; i < 33; i++) {
			result += texture(bloomSampler, uv + vec2(0.0, texelSize.y * offsets[i])).rgb * weights[i];
		}
	}

	outColor = vec4(result, 1.0);
}