#version 460
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_DIR_LIGHTS 10
#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS 10

#define MAX_REFLECTION_LOD 4.0

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

layout(push_constant) uniform MaterialIndices {
	int diffuseIndex;
	int normalIndex;
	int metallicRoughnessIndex;
	int emissiveIndex;
	int occlusionIndex;
} mI;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 accumulationColor;
layout(location = 1) out float revealageColor;

#define M_PI 3.1415926535897932384626433832795

void main() {
	vec4 colorSample = texture(textures[mI.diffuseIndex], uv);
	vec3 tmpColor = colorSample.rgb;

	vec4 premultiplied = vec4(tmpColor * colorSample.a, colorSample.a);
	float a = min(1.0, premultiplied.a) * 8.0 + 0.01;
	float b = -gl_FragCoord.z * 0.95 + 1.0;
	float w = clamp(a * a * a * 1e3 * b * b * b, 1e-2, 3e2);
	accumulationColor = premultiplied * w;
	revealageColor = colorSample.a;
}