#version 460
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable

#include "../shaders/wboit/weightsFunctions.glsl"

#define MAX_DIR_LIGHTS 10
#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS 10

struct PerDrawMaterial {
	int materialIndex;
	float alphaCutoff;
};

layout(constant_id = 0) const int alphaMode = 0;

layout(set = 0, binding = 3) uniform Lighting {
	vec3 numLights;
	vec3 dirLightsDirection[MAX_DIR_LIGHTS];
	vec3 dirLightsColor[MAX_DIR_LIGHTS];
	vec3 pointLightsPosition[MAX_POINT_LIGHTS];
	vec3 pointLightsColor[MAX_POINT_LIGHTS];
	vec3 spotLightsPosition[MAX_SPOT_LIGHTS];
	vec3 spotLightsDirection[MAX_SPOT_LIGHTS];
	vec3 spotLightsColor[MAX_SPOT_LIGHTS];
	vec2 spotLightsCutoffs[MAX_SPOT_LIGHTS];
} lights;

layout(set = 0, binding = 4) uniform samplerCube irradianceMap;
layout(set = 0, binding = 5) uniform samplerCube prefilterMap;
layout(set = 0, binding = 6) uniform sampler2D brdfLUT;

layout(set = 0, binding = 7) uniform sampler2DShadow shadowMaps[MAX_DIR_LIGHTS + MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS];

layout(set = 1, binding = 0) uniform sampler2D textures[];
layout(set = 1, binding = 1) restrict readonly buffer Materials {
	int diffuseIndex;
	int normalIndex;
	int metallicRoughnessIndex;
	int emissiveIndex;
	int occlusionIndex;
} materials[];

layout(set = 2, binding = 0) restrict readonly buffer PerDraw {
	PerDrawMaterial perDrawMaterial[];
} perDraw;

layout(location = 0) in vec2 uv;
layout(location = 1) in flat int drawIndex;

layout(location = 0) out vec4 outputColor0;
layout(location = 1) out float outputColor1;

void main() {
	vec4 colorSample = texture(textures[materials[perDraw.perDrawMaterial[drawIndex].materialIndex].diffuseIndex], uv);
	if (alphaMode == 1) {
		if (colorSample.w <= perDraw.perDrawMaterial[drawIndex].alphaCutoff) {
			discard;
		}
	}
	
	if (alphaMode == 0 || alphaMode == 1) {
		outputColor0 = vec4(colorSample.rgb, 1.0);
	}
	else if (alphaMode == 2) {
		vec4 premultiplied = vec4(tmpColor * colorSample.a, colorSample.a);
		float w = w1();
		outputColor0 = premultiplied * w;
		outputColor1 = colorSample.a;
	}
}