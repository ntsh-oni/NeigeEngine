#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../src/graphics/shaders/atmosphere/atmosphereFunctions.glsl"

layout(set = 0, binding = 2) uniform sampler2D skyViewLUT;

layout(set = 0, binding = 3) uniform sampler2D depthPrepass;

layout(set = 0, binding = 4) uniform sampler3D cameraVolume;

layout(set = 0, binding = 5) uniform Camera {
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
    vec3 sunDir = normalize(-pC.lightDir);

    float viewHeight = length(worldPos);
    vec3 l = vec3(0.0);
    float depthBufferValue = texture(depthPrepass, uv).r;
    
    if (viewHeight < topRadius && depthBufferValue == 1.0) {
        vec3 upVector = normalize(worldPos);
        float viewZenithCosAngle = dot(worldDir, upVector);

        vec3 sideVector = normalize(cross(upVector, worldDir));
        vec3 forwardVector = normalize(cross(sideVector, upVector));
        vec2 lightOnPlane = vec2(dot(sunDir, forwardVector), dot(sunDir, sideVector));
        lightOnPlane = normalize(lightOnPlane);
        float lightViewCosAngle = lightOnPlane.x;

        bool intersectGround = raySphereIntersectNearest(worldPos, worldDir, vec3(0.0), bottomRadius) >= 0.0;

        vec2 tmpUV = skyViewLutParamsToUV(intersectGround, viewZenithCosAngle, lightViewCosAngle, viewHeight);

        outColor = vec4(texture(skyViewLUT, tmpUV).rgb + getSunLuminance(worldPos, worldDir, bottomRadius, sunDir), 1.0);
    }
    else {
        clipSpace = vec3(uv * 2.0 - 1.0, depthBufferValue);
        vec4 depthBufferWorldPos = invViewProj * vec4(clipSpace, 1.0);
        depthBufferWorldPos /= depthBufferWorldPos.w;
        float tDepth = length(depthBufferWorldPos.xyz - (worldPos + vec3(0.0, -bottomRadius, 0.0)));
        float slice = tDepth * (1.0 / 4.0);
        float weight = 1.0;
        if (slice < 0.5) {
            weight = clamp(slice * 2.0, 0.0, 1.0);
            slice = 0.5;
        }
        float w = sqrt(slice / 32.0);

        const vec4 ap = weight * texture(cameraVolume, vec3(uv, w));
        l += ap.rgb;
        float opacity = ap.a;

        outColor = vec4(l, opacity);
    }
}