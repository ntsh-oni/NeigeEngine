#version 460

layout(set = 0, binding = 0) uniform sampler2D rayMarching;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 rayMarchingSample = texture(rayMarching, uv);
    rayMarchingSample /= rayMarchingSample.a;

    vec3 whiteDot = vec3(1.08241, 0.92756, 0.95003);
    float exposure = 10.0;

    outColor = vec4(pow(vec3(1.0) - exp(-rayMarchingSample.rgb / whiteDot * exposure), vec3(1.0 / 2.0)), 1.0);
}