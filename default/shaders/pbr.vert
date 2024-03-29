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

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 color;
layout(location = 4) in vec4 tangent;

layout(location = 0) out vec2 outUv;
layout(location = 1) out vec3 outCameraPos;
layout(location = 2) out vec3 outFragmentPos;
layout(location = 3) out int outDrawIndex;
layout(location = 4) out vec4 outDirLightSpaces[MAX_DIR_LIGHTS];
layout(location = MAX_DIR_LIGHTS + 4) out vec4 outSpotLightSpaces[MAX_SPOT_LIGHTS];
layout(location = MAX_DIR_LIGHTS + MAX_SPOT_LIGHTS + 4) out mat3 outTBN;

void main() {
	outUv = uv;
	vec3 bitangent = cross(normal, tangent.xyz) * tangent.w;
	vec3 T = vec3(object.model * vec4(tangent.xyz, 0.0));
	vec3 B = vec3(object.model * vec4(bitangent, 0.0));
	vec3 N = vec3(object.model * vec4(normal, 0.0));
	outTBN = mat3(T, B, N);
	outCameraPos = camera.pos;
	outFragmentPos = vec3(object.model * vec4(position, 1.0));
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