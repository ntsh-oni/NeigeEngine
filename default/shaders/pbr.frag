#version 460
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable

#include "../src/graphics/shaders/wboit/weightFunctions.glsl"

#define MAX_DIR_LIGHTS 10
#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS 10

#define MAX_REFLECTION_LOD 4.0

struct PerDrawMaterial {
	int materialIndex;
	float alphaCutoff;
};

layout(constant_id = 0) const int alphaMode = 0;

layout(set = 0, binding = 3) uniform Lighting {
	vec3 numLights;
	vec4 dirLightsDirectionAndShadowIndex[MAX_DIR_LIGHTS];
	vec3 dirLightsColor[MAX_DIR_LIGHTS];
	vec3 pointLightsPosition[MAX_POINT_LIGHTS];
	vec3 pointLightsColor[MAX_POINT_LIGHTS];
	vec4 spotLightsPositionAndShadowIndex[MAX_SPOT_LIGHTS];
	vec3 spotLightsDirection[MAX_SPOT_LIGHTS];
	vec3 spotLightsColor[MAX_SPOT_LIGHTS];
	vec2 spotLightsCutoffs[MAX_SPOT_LIGHTS];
} lights;

layout(set = 0, binding = 4) uniform samplerCube irradianceMap;
layout(set = 0, binding = 5) uniform samplerCube prefilterMap;
layout(set = 0, binding = 6) uniform sampler2D brdfLUT;

layout(set = 0, binding = 7) uniform sampler2DShadow directionalLightsShadowMaps[MAX_DIR_LIGHTS];
layout(set = 0, binding = 8) uniform sampler2DShadow spotLightsShadowMaps[MAX_SPOT_LIGHTS];

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
layout(location = 1) in vec3 cameraPos;
layout(location = 2) in vec3 fragmentPos;
layout(location = 3) in flat int drawIndex;
layout(location = 4) in vec4 dirLightSpaces[MAX_DIR_LIGHTS];
layout(location = MAX_DIR_LIGHTS + 4) in vec4 spotLightSpaces[MAX_SPOT_LIGHTS];
layout(location = MAX_DIR_LIGHTS + MAX_SPOT_LIGHTS + 4) in mat3 TBN;

layout(location = 0) out vec4 outputColor0;
layout(location = 1) out float outputColor1;

#define M_PI 3.1415926535897932384626433832795

float distribution(float NdotH, float roughness) {
	float a = roughness * roughness;
	float asquare = a * a;
	float NdotHsquare = NdotH * NdotH;
	float denom = NdotHsquare * (asquare - 1.0) + 1.0;

	return asquare / (M_PI * denom * denom);
}

vec3 fresnel(float costheta, vec3 f0) {
	return f0 + (1.0 - f0) * pow(1.0 - costheta, 5.0);
}

vec3 fresnelRoughness(float costheta, vec3 f0, float roughness) {
	return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - costheta, 5.0);
}

float g(float NdotV, float roughness) {
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;
	float denom = NdotV * (1.0 - k) + k;

	return NdotV / denom;
}

float smith(float LdotN, float VdotN, float roughness) {
	float gv = g(VdotN, roughness);
	float gl = g(LdotN, roughness);

	return gv * gl;
}

vec3 diffuseFresnelCorrection(vec3 ior) {
	vec3 iorsquare = ior * ior;
	bvec3 TIR = lessThan(ior, vec3(1.0));
	vec3 invdenum = mix(vec3(1.0), vec3(1.0) / (iorsquare * iorsquare * (vec3(554.33) - 380.7 * ior)), TIR);
	vec3 num = ior * mix(vec3(0.1921156102251088), ior * 298.25 - 261.38 * iorsquare + 138.43, TIR);
	num += mix(vec3(0.8078843897748912), vec3(-1.67), TIR);

	return num * invdenum;
}

vec3 brdf(float LdotH, float NdotH, float VdotH, float LdotN, float VdotN, vec3 diffuse, float metallic, float roughness) {
	float d = distribution(NdotH, roughness);
	vec3 f = fresnel(LdotH, mix(vec3(0.04), diffuse, metallic));
	vec3 fT = fresnel(LdotN, mix(vec3(0.04), diffuse, metallic));
	vec3 fTIR = fresnel(VdotN, mix(vec3(0.04), diffuse, metallic));
	float g = smith(LdotN, VdotN, roughness);
	vec3 dfc = diffuseFresnelCorrection(vec3(1.05));

	vec3 lambertian = diffuse / M_PI;

	return (d * f * g) / max(4.0 * LdotN * VdotN, 0.001) + ((vec3(1.0) - fT) * (vec3(1.0) - fTIR) * lambertian) * dfc;
}

vec3 shade(vec3 n, vec3 v, vec3 l, vec3 lc, vec3 diffuse, float metallic, float roughness) {
	vec3 h = normalize(v + l);
	float LdotH = max(dot(l, h), 0.0);
	float NdotH = max(dot(n, h), 0.0);
	float VdotH = max(dot(v, h), 0.0);
	float LdotN = max(dot(l, n), 0.0);
	float VdotN = max(dot(v, n), 0.0);
	vec3 brdf = brdf(LdotH, NdotH, VdotH, LdotN, VdotN, diffuse, metallic, roughness);
	vec3 ret = lc * brdf * LdotN;

	return ret;
}

float shadowValue(vec4 lightSpace, sampler2DShadow shadowMap) {
	vec3 proj = lightSpace.xyz / lightSpace.w;
	if (proj.z > 1.0) {
		return 1.0;
	}
	proj = proj * 0.5 + 0.5;
	float curr = proj.z - 0.005;
	
	float shadow = 0.0;
	
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			vec3 shadowUvs = vec3(vec2(proj.xy) + vec2(x, y) * texelSize, curr);
			shadow += texture(shadowMap, shadowUvs).r;
		}
	}
	
	return shadow / 9.0;
}

void main() {
	vec4 colorSample = texture(textures[materials[perDraw.perDrawMaterial[drawIndex].materialIndex].diffuseIndex], uv);
	if (alphaMode == 1) {
		if (colorSample.w <= perDraw.perDrawMaterial[drawIndex].alphaCutoff) {
			discard;
		}
	}
	vec3 normalSample = texture(textures[materials[perDraw.perDrawMaterial[drawIndex].materialIndex].normalIndex], uv).xyz;
	float metallicSample = texture(textures[materials[perDraw.perDrawMaterial[drawIndex].materialIndex].metallicRoughnessIndex], uv).b;
	float roughnessSample = texture(textures[materials[perDraw.perDrawMaterial[drawIndex].materialIndex].metallicRoughnessIndex], uv).g;
	vec3 emissiveSample = texture(textures[materials[perDraw.perDrawMaterial[drawIndex].materialIndex].emissiveIndex], uv).xyz;
	float occlusionSample = texture(textures[materials[perDraw.perDrawMaterial[drawIndex].materialIndex].occlusionIndex], uv).r;
	
	vec3 d = vec3(colorSample);
	vec3 n = normalSample * 2.0 - 1.0;
	n = normalize(TBN * n);
	vec3 v = normalize(cameraPos - fragmentPos);
	vec3 r = reflect(-v, n);
	
	vec3 l;

	vec3 tmpColor = vec3(0.0);
	
	int numDirLights = int(lights.numLights.x);
	int numPointLights = int(lights.numLights.y);
	int numSpotLights = int(lights.numLights.z);
	
	float shadow = 0.0;
	for (int i = 0; i < numDirLights; i++) {
		l = normalize(-lights.dirLightsDirectionAndShadowIndex[i].xyz);
		shadow = shadowValue(dirLightSpaces[i], directionalLightsShadowMaps[int(lights.dirLightsDirectionAndShadowIndex[i].w)]);
		tmpColor += shade(n, v, l, lights.dirLightsColor[i], d, metallicSample, roughnessSample) * shadow;
	}

	for (int i = 0; i < numPointLights; i++) {
		l = normalize(lights.pointLightsPosition[i] - fragmentPos);
		float distance = length(lights.pointLightsPosition[i] - fragmentPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lights.pointLightsColor[i] * attenuation;
		tmpColor += shade(n, v, l, radiance, d, metallicSample, roughnessSample);
	}

	for (int i = 0; i < numSpotLights; i++) {
		l = normalize(lights.spotLightsPositionAndShadowIndex[i].xyz - fragmentPos);
		float theta = dot(l, normalize(-lights.spotLightsDirection[i]));
		if (theta > lights.spotLightsCutoffs[i].x) {
			float shadow = shadowValue(spotLightSpaces[i], spotLightsShadowMaps[int(lights.spotLightsPositionAndShadowIndex[i].w)]);
			tmpColor += shade(n, v, l, lights.spotLightsColor[i], d, metallicSample, roughnessSample) * shadow;
		}
		else if (theta > lights.spotLightsCutoffs[i].y) {
			float shadow = shadowValue(spotLightSpaces[i], spotLightsShadowMaps[int(lights.spotLightsPositionAndShadowIndex[i].w)]);
			float epsilon = lights.spotLightsCutoffs[i].x - lights.spotLightsCutoffs[i].y;
			float intensity = clamp((theta - lights.spotLightsCutoffs[i].y) / epsilon, 0.0, 1.0);
			tmpColor += shade(n, v, l, lights.spotLightsColor[i] * intensity, d * intensity, metallicSample, roughnessSample) * shadow;
		}
	}

	vec3 fRoughness = fresnelRoughness(max(dot(n, v), 0.0), mix(vec3(0.04), d, metallicSample), roughnessSample);
	vec3 irradianceDiffuse = 1.0 - fRoughness;
	irradianceDiffuse *= 1.0 - metallicSample;
	vec3 irradianceSample = vec3(texture(irradianceMap, n));
	vec3 diffuse = irradianceSample * d;
	
	vec3 prefilterSample = vec3(textureLod(prefilterMap, r, roughnessSample * MAX_REFLECTION_LOD));
	vec2 envBrdf = vec2(texture(brdfLUT, vec2(max(dot(n, v), 0.0), roughnessSample)));
	vec3 specular = prefilterSample * (fRoughness * envBrdf.x + envBrdf.y);
	
	vec3 ambient = (irradianceDiffuse * diffuse + specular) * occlusionSample;
	tmpColor += ambient;
	
	tmpColor += emissiveSample * 2.0;
	
	if (alphaMode == 0 || alphaMode == 1) {
		outputColor0 = vec4(tmpColor, 1.0);
	}
	else if (alphaMode == 2) {
		vec4 premultiplied = vec4(tmpColor * colorSample.a, colorSample.a);
		float w = w1();
		outputColor0 = premultiplied * w;
		outputColor1 = colorSample.a;
	}
}