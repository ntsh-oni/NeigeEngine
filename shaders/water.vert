#version 460

#define MAX_DIR_LIGHTS 10
#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS 10

layout(set = 0, binding = 0) uniform Object {
	mat4 model;
} object;

layout(set = 0, binding = 1) uniform Camera {
	mat4 view;
	mat4 projection;
	vec3 pos;
} camera;

layout(set = 0, binding = 2) uniform Shadow {
	vec3 numLights;
	mat4 dirLightSpaces[MAX_DIR_LIGHTS];
	mat4 spotLightSpaces[MAX_SPOT_LIGHTS];
} shadow;

layout(set = 0, binding = 8) uniform Time {
	float time;
} time;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 outUv;
layout(location = 1) out vec3 outCameraPos;
layout(location = 2) out vec3 outFragmentPos;
layout(location = 3) out int outDrawIndex;
layout(location = 4) out vec4 outDirLightSpaces[MAX_DIR_LIGHTS];
layout(location = MAX_DIR_LIGHTS + 4) out vec4 outSpotLightSpaces[MAX_SPOT_LIGHTS];
layout(location = MAX_DIR_LIGHTS + MAX_SPOT_LIGHTS + 4) out mat3 outTBN;

#define M_PI 3.1415926535897932384626433832795
#define WAVE0 vec4(1.0, 1.0, 0.05, 0.5)
#define WAVE1 vec4(-1.0, 1.0, 0.25, 0.5)
#define WAVE2 vec4(1.0, 0.0, 0.05, 0.5)

vec3 gerstnerWave(vec4 wave, vec3 p, inout vec3 tangent, inout vec3 binormal) {
	float steepness = wave.z;
	float wavelength = wave.w;
	float k = 2 * M_PI / wavelength;
	float c = sqrt(9.8 / k);
	vec2 d = normalize(wave.xy);
	float f = (k * (dot(d, p.xz) - c * time.time));
	float a = steepness / k;
	
	tangent += vec3(-d.x * d.x * (steepness * sin(f)), d.x * steepness * cos(f), -d.x * d.y * (steepness * sin(f)));
	binormal += vec3(-d.x * d.y * (steepness * sin(f)), d.y * (steepness * cos(f)), -d.y * d.y * (steepness * sin(f)));
	return vec3(d.x * (a * cos(f)), a * sin(f), d.y * (a * cos(f)));
}

void main() {
	outUv = uv;
	outCameraPos = camera.pos;
	
	vec3 tangent = vec3(1.0, 0.0, 0.0);
	vec3 binormal = vec3(0.0, 0.0, 1.0);
	vec3 newPosition = position;
	newPosition += gerstnerWave(WAVE0, position, tangent, binormal);
	newPosition += gerstnerWave(WAVE1, position, tangent, binormal);
	newPosition += gerstnerWave(WAVE2, position, tangent, binormal);
		
	outFragmentPos = vec3(object.model * vec4(newPosition, 1.0));
	vec3 normal = normalize(cross(binormal, tangent));
	vec3 bitangent = cross(normal, tangent.xyz);
	vec3 T = vec3(object.model * vec4(tangent.xyz, 0.0));
	vec3 B = vec3(object.model * vec4(bitangent, 0.0));
	vec3 N = vec3(object.model * vec4(normal, 0.0));
	outTBN = mat3(T, B, N);
	
	outDrawIndex = gl_DrawID;

	int numDirLights = int(shadow.numLights.x);
	int numPointLights = int(shadow.numLights.y);
	int numSpotLights = int(shadow.numLights.z);
	
	for (int i = 0; i < shadow.numLights.x; i++) {
		outDirLightSpaces[i] = shadow.dirLightSpaces[i] * vec4(outFragmentPos, 1.0);
	}
	
	for (int i = 0; i < shadow.numLights.z; i++) {
		outSpotLightSpaces[i] = shadow.spotLightSpaces[i] * vec4(outFragmentPos, 1.0);
	}
	
	gl_Position = camera.projection * camera.view * vec4(outFragmentPos, 1.0);
}