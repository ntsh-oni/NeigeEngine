#version 450

layout(set = 0, binding = 0) uniform sampler2D postSampler;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

const float SPAN_MAX = 8.0;
const float REDUCE_MUL = 1.0 / 8.0;
const float REDUCE_MIN = 1.0 / 128.0;

void main() {
	vec2 texSize = vec2(textureSize(postSampler, 0));
	vec2 texelSize = 1.0 / texSize;
	
	vec3 nw = texture(postSampler, uv + (vec2(-1.0, -1.0) * texelSize)).rgb;
	vec3 ne = texture(postSampler, uv + (vec2(1.0, -1.0) * texelSize)).rgb;
	vec3 sw = texture(postSampler, uv + (vec2(-1.0, 1.0) * texelSize)).rgb;
	vec3 se = texture(postSampler, uv + (vec2(1.0, 1.0) * texelSize)).rgb;
	vec3 m = texture(postSampler, uv).rgb;
	
	vec3 brightnessCoefficients = vec3(0.2126, 0.7152, 0.0722);
	float brightnessNW = dot(nw, brightnessCoefficients);
	float brightnessNE = dot(ne, brightnessCoefficients);
	float brightnessSW = dot(sw, brightnessCoefficients);
	float brightnessSE = dot(se, brightnessCoefficients);
	float brightnessM = dot(m, brightnessCoefficients);
	
	float brightnessMin = min(brightnessM, min(min(brightnessNW, brightnessNE), min(brightnessSW, brightnessSE)));
	float brightnessMax = max(brightnessM, max(max(brightnessNW, brightnessNE), max(brightnessSW, brightnessSE)));
	
	vec2 dir;
	dir.x = -((brightnessNW + brightnessNE) - (brightnessSW + brightnessSE));
	dir.y = (brightnessNW + brightnessSW) - (brightnessNE + brightnessSE);
	
	float reduce = max((brightnessNW + brightnessNE + brightnessSW + brightnessSE) * (0.25 * REDUCE_MUL), REDUCE_MIN);
	float dirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + reduce);
	
	dir = min(vec2(SPAN_MAX), max(vec2(-SPAN_MAX), dir * dirMin)) / texSize;
	
	vec3 a = (1.0 / 2.0) * (texture(postSampler, uv + dir * (1.0 / 3.0 - 0.5)).rgb + texture(postSampler, uv + dir * (2.0 / 3.0 - 0.5)).rgb);
	vec3 b = (1.0 / 2.0) + (1.0 / 4.0) * (texture(postSampler, uv + dir * -0.5).rgb + texture(postSampler, uv + dir * 0.5).rgb);
	float brightness = dot(b, brightnessCoefficients);
	
	if ((brightness < brightnessMin) || (brightness > brightnessMax)) {
		outColor = vec4(a, 1.0);
	}
	else {
		outColor = vec4(b, 1.0);
	}
}