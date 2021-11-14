#version 460

#define MAX_DIR_LIGHTS 10
#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS 10

layout(set = 0, binding = 0) uniform Object {
	mat4 model;
} object;

layout(set = 0, binding = 1) uniform Shadow {
	vec3 numLights;
	mat4 dirLightSpaces[MAX_DIR_LIGHTS];
	mat4 spotLightSpaces[MAX_SPOT_LIGHTS];
} shadow;

layout(push_constant) uniform PushConstants {
	layout(offset = 0) int shadowIndex;
	int lightType;
} pC;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 outUv;
layout(location = 1) out int outDrawIndex;

void main() {
	outUv = uv;
	outDrawIndex = gl_DrawID;

	int numDirLights = int(shadow.numLights.x);
	
	if (pC.lightType == 0) {
		gl_Position = shadow.dirLightSpaces[pC.shadowIndex - 1] * object.model * vec4(position, 1.0);
	}
	else if (pC.lightType == 2) {
		gl_Position = shadow.spotLightSpaces[pC.shadowIndex - 1] * object.model * vec4(position, 1.0);
	}

	gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}