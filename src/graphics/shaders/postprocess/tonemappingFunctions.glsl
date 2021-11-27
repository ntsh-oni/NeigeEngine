vec3 reinhard(vec3 color) {
	return color / (color + vec3(1.0));
}

vec3 reinhardExtended(vec3 color, vec3 whitePoint) {
	return (color * (1.0 + (color / (whitePoint * whitePoint)))) / (color + vec3(1.0));
}

vec3 reinhardExtendedLuminance(vec3 color, float whitePointLuminance) {
	float colorLuminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float n = colorLuminance * (1.0 + (colorLuminance / (whitePointLuminance * whitePointLuminance)));
	float newLuminance = n / (1.0 + colorLuminance);
	return color * (newLuminance / colorLuminance);
}

vec3 filmic(vec3 color) {
	const float exposureBias = 2.0;
	vec3 colorExposureBias = color * 2.0;
	
	const float A = 0.15;
	const float B = 0.50;
	const float C = 0.10;
	const float D = 0.20;
	const float E = 0.02;
	const float F = 0.30;
	vec3 current = ((colorExposureBias * (A * colorExposureBias + C * B) + D * E) / (colorExposureBias * (A * colorExposureBias + B) + D * F)) - E / F;

	const vec3 W = vec3(11.2);
	vec3 whiteScale = vec3(1.0) / (((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F);

	return current * whiteScale;
}

vec3 aces(vec3 color) {
	const mat3 inputMatrix = mat3(vec3(0.59719, 0.07600, 0.02840), vec3(0.35458, 0.90834, 0.13383), vec3(0.04823, 0.01566, 0.83777));
	const mat3 outputMatrix = mat3(vec3(1.60475, -0.10208, -0.00327), vec3(-0.53108, 1.10813, -0.07276), vec3(-0.07367, -0.00605, 1.07602));

	vec3 newColor = inputMatrix * color;

	vec3 a = newColor * (newColor + 0.0245786) - 0.000090537;
	vec3 b = newColor * (0.983729 * newColor + 0.4329510) + 0.238081;
	newColor = a / b;
	
	return outputMatrix * newColor;
}

vec3 acesApproximation(vec3 color) {
	vec3 newColor = color * 0.6;

	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;

	return clamp((newColor * (A * newColor + B)) / (newColor * (C * newColor + D) + E), 0.0, 1.0);
}