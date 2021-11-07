#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../src/graphics/shaders/atmosphere/atmosphereFunctions.glsl"

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
    float x_mu = uv.x;
    float x_r = uv.y;

    float h = sqrt((topRadius * topRadius) - (bottomRadius * bottomRadius));
    float rho = h * x_r;
    float viewHeight = sqrt((rho * rho) + (bottomRadius * bottomRadius));

    float d_min = topRadius - viewHeight;
    float d_max = rho + h;
    float d = d_min + x_mu * (d_max - d_min);
    float viewZenithCosAngle = (d == 0.0) ? 1.0f : (((h * h) - (rho * rho) - (d * d)) / (2.0 * viewHeight * d));
    viewZenithCosAngle = clamp(viewZenithCosAngle, -1.0, 1.0);

    vec3 worldPos = vec3(0.0, viewHeight, 0.0);
    vec3 worldDir = vec3(0.0, viewZenithCosAngle, sqrt(1.0 - (viewZenithCosAngle * viewZenithCosAngle)));

    const bool ground = false;
    const float sampleCountIni = 40.0;
    const float depthBufferValue = -1.0;
    const bool variableSampleCount = false;
    const bool mieRayPhase = false;

    vec3 transmittance = exp(-integrateScatteredLuminance(vec2(0.0), mat4(0.0), worldPos, worldDir, vec3(0.0), ground, sampleCountIni, depthBufferValue, variableSampleCount, mieRayPhase, 9000000.0).opticalDepth);
    
    outColor = vec4(transmittance, 1.0);
}