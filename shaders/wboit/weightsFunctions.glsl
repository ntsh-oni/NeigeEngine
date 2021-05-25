float w0(float alpha) {
	float a = min(1.0, alpha) * 8.0 + 0.01;
	float b = -gl_FragCoord.z * 0.95 + 1.0;
	return clamp(a * a * a * 1e3 * b * b * b, 1e-2, 3e2);
}

float w1() {
	return max(pow(clamp(1.0 - gl_FragCoord.z, 0.0, 1.0), 3.0) * 3000.0, 0.01);
}

float w2() {
	return max(min(0.03 / (pow(abs(gl_FragCoord.z) / 200.0, 4.0) + 0.00001), 3000.0), 0.01);
}

float w3() {
	return max(min(10.0 / (pow(abs(gl_FragCoord.z) / 200.0, 6.0) + pow(abs(gl_FragCoord.z) / 10.0, 3.0) + 0.00001), 3000.0), 0.01);
}

float w4() {
	return max(min(10.0 / (pow(abs(gl_FragCoord.z) / 200.0, 6.0) + pow(abs(gl_FragCoord.z) / 5.0, 2.0) + 0.00001), 3000.0), 0.01);
}