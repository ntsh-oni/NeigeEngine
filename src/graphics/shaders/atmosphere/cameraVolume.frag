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
layout(location = 1) in flat int instanceID;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 clipSpace = vec3(uv * 2.0 - 1.0, 0.5);
    mat4 invViewProj = inverse(camera.projection * camera.view);
    vec4 hPos = invViewProj * vec4(clipSpace, 1.0);
    vec3 worldDir = normalize(hPos.xyz / hPos.w - camera.pos);
    float planetR = bottomRadius;
    vec3 planetO = vec3(0.0, -planetR, 0.0);
    vec3 camPos = camera.pos + vec3(0.0, planetR, 0.0);
    vec3 sunDir = normalize(-pC.lightDir);
    vec3 sunLuminance = vec3(0.0);

    float slice = (float(instanceID) + 0.5) / 32.0;
    slice *= slice;
    slice *= 32.0;

    vec3 worldPos = camPos;
    float viewHeight;

    float tMax = slice * 4.0;
    vec3 newWorldPos = worldPos + tMax * worldDir;

    viewHeight = length(newWorldPos);
    if (viewHeight <= (bottomRadius + planetRadiusOffset)) {
        newWorldPos = normalize(newWorldPos) * (bottomRadius + planetRadiusOffset + 0.001);
        worldDir = normalize(newWorldPos - camPos);
        tMax = length(newWorldPos - camPos);
    }
    float tMaxMax = tMax;

    bool gotResult = false;
    viewHeight = length(worldPos);
    if (viewHeight >= topRadius) {
        vec3 prevWorldPos = worldPos;
        vec4 moveToTop = moveToTopAtmosphere(worldPos, worldDir);
        worldPos = moveToTop.xyz;
        if (moveToTop.w == 0.0) {
            outColor = vec4(0.0, 0.0, 0.0, 1.0);
            gotResult = true;
        }
        float lengthToAtmosphere = length(prevWorldPos - worldPos);
        if (tMaxMax < lengthToAtmosphere) {
            outColor = vec4(0.0, 0.0, 0.0, 1.0);
            gotResult = true;
        }
        tMaxMax = max(0.0, tMaxMax - lengthToAtmosphere);
    }

    if (!gotResult) {
        const bool ground = false;
        const float sampleCountIni = max(1.0, float(instanceID + 1.0) * 2.0);
        const float depthBufferValue = -1.0;
        const bool variableSampleCount = false;
        const bool mieRayPhase = true;
        SingleScatteringResult result = integrateScatteredLuminance(vec2(0.0), mat4(0.0), worldPos, worldDir, sunDir, ground, sampleCountIni, depthBufferValue, variableSampleCount, mieRayPhase, tMaxMax);

        const float transmittance = dot(result.transmittance, vec3(1.0 / 3.0));
        outColor = vec4(result.l, 1.0 - transmittance);
    }
}