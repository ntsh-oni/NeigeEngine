#version 460

layout(set = 0, binding = 0) uniform sampler2D postSampler;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

const float THRESHOLD = 0.0312;
const float RELATIVE_THRESHOLD = 0.125;

void main() {
	vec2 texSize = vec2(textureSize(postSampler, 0));
	vec2 texelSize = 1.0 / texSize;
	
	vec3 n	= texture(postSampler, uv + (vec2(0.0, -1.0) * texelSize)).rgb;
	vec3 s = texture(postSampler, uv + (vec2(0.0, 1.0) * texelSize)).rgb;
	vec3 e = texture(postSampler, uv + (vec2(1.0, 0.0) * texelSize)).rgb;
	vec3 w = texture(postSampler, uv + (vec2(-1.0, 0.0) * texelSize)).rgb;
	vec3 m = texture(postSampler, uv).rgb;
	
	vec3 brightnessCoefficients = vec3(0.2126, 0.7152, 0.0722);
	float brightnessN = dot(n, brightnessCoefficients);
	float brightnessS = dot(s, brightnessCoefficients);
	float brightnessE = dot(e, brightnessCoefficients);
	float brightnessW = dot(w, brightnessCoefficients);
	float brightnessM = dot(m, brightnessCoefficients);
	
	float brightnessMin = min(brightnessM, min(min(brightnessN, brightnessS), min(brightnessE, brightnessW)));
	float brightnessMax = max(brightnessM, max(max(brightnessN, brightnessS), max(brightnessE, brightnessW)));
	
	float contrast = brightnessMax - brightnessMin;
	float threshold = max(THRESHOLD, RELATIVE_THRESHOLD * brightnessMax);
	
	if (contrast < threshold) {
		outColor = vec4(m, 1.0);
	}
	else {
		vec3 nw = texture(postSampler, uv + (vec2(-1.0, -1.0) * texelSize)).rgb;
		vec3 ne = texture(postSampler, uv + (vec2(1.0, -1.0) * texelSize)).rgb;
		vec3 sw = texture(postSampler, uv + (vec2(-1.0, 1.0) * texelSize)).rgb;
		vec3 se = texture(postSampler, uv + (vec2(1.0, 1.0) * texelSize)).rgb;
		
		float brightnessNW = dot(nw, brightnessCoefficients);
		float brightnessNE = dot(ne, brightnessCoefficients);
		float brightnessSW = dot(sw, brightnessCoefficients);
		float brightnessSE = dot(se, brightnessCoefficients);
		
		float factor = 2 * (brightnessN + brightnessS + brightnessE + brightnessW);
		factor += (brightnessNW + brightnessNE + brightnessSW + brightnessSE);
		factor *= (1.0 / 12.0);
		factor = abs(factor - brightnessM);
		factor = clamp(factor / contrast, 0.0, 1.0);
		factor = smoothstep(0.0, 1.0, factor);
		factor = factor * factor;
		
		float horizontal = abs(brightnessN + brightnessS - (2 * brightnessM)) * 2 +
			abs(brightnessNE + brightnessSE - (2 * brightnessE)) +
			abs(brightnessNW + brightnessSW - (2 * brightnessW));
		float vertical = abs(brightnessE + brightnessW - (2 * brightnessM)) * 2 +
			abs(brightnessNE + brightnessSE - (2 * brightnessN)) +
			abs(brightnessNW + brightnessSW - (2 * brightnessS));
		bool isHorizontal = horizontal > vertical;
		
		float pixelStep = isHorizontal ? texelSize.y : texelSize.x;
		
		float posBrightness = isHorizontal ? brightnessS : brightnessE;
		float negBrightness = isHorizontal ? brightnessN : brightnessW;
		float posGradient = abs(posBrightness - brightnessM);
		float negGradient = abs(negBrightness - brightnessM);
		
		pixelStep *= (posGradient < negGradient) ? -1 : 1;
		
		vec2 blendUV = uv;
		if (isHorizontal) {
			blendUV.y = uv.y + (pixelStep * factor);
		}
		else {
			blendUV.x = uv.x + (pixelStep * factor); 
		}
		
		outColor = texture(postSampler, blendUV);
	}
}