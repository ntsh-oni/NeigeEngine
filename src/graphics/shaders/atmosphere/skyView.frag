#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../src/graphics/shaders/atmosphere/atmosphereFunctions.glsl"

layout(set = 0, binding = 2) uniform Camera {
	mat4 view;
	mat4 projection;
	vec3 pos;
} camera;

layout(push_constant) uniform PushConstants {
    vec3 lightDir;
} pC;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 clipSpace = vec3(uv * 2.0 - 1.0, 1.0);
    mat4 invViewProj = inverse(camera.projection * camera.view);
    vec4 hPos = invViewProj * vec4(clipSpace, 1.0);
    vec3 worldDir = normalize(hPos.xyz / hPos.w - camera.pos);
    vec3 worldPos = camera.pos + vec3(0.0, bottomRadius, 0.0);
    vec3 sunDirec = normalize(-pC.lightDir);

    float viewHeight = length(worldPos);

    vec2 skyViewLutParams = uvToSkyViewLutParams(viewHeight, uv);
    float viewZenithCosAngle = skyViewLutParams.x;
    float lightViewCosAngle = skyViewLutParams.y;

    vec3 sunDir;
    vec3 upVector = worldPos / viewHeight;
    float sunZenithCosAngle = dot(upVector, sunDirec);
    sunDir = normalize(vec3(sqrt(1.0 - (sunZenithCosAngle * sunZenithCosAngle)), sunZenithCosAngle, 0.0));

    worldPos = vec3(0.0, viewHeight, 0.0);

    float viewZenithSinAngle = sqrt(1.0 - viewZenithCosAngle * viewZenithCosAngle);
    worldDir = vec3(viewZenithSinAngle * lightViewCosAngle,
    viewZenithCosAngle,
    viewZenithSinAngle * sqrt(1.0 - (lightViewCosAngle * lightViewCosAngle)));

    vec4 moveToTop = moveToTopAtmosphere(worldPos, worldDir);
    worldPos = moveToTop.xyz;
    
    if (moveToTop.w == 0.0) {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else {
        const bool ground = false;
        const float sampleCountIni = 30.0;
        const float depthBufferValue = -1.0;
        const bool variableSampleCount = true;
        const bool mieRayPhase = true;
        SingleScatteringResult result = integrateScatteredLuminance(vec2(0.0), mat4(0.0), worldPos, worldDir, sunDir, ground, sampleCountIni, depthBufferValue, variableSampleCount, mieRayPhase, 9000000.0);

        outColor = vec4(result.l, 1.0);
    }
}