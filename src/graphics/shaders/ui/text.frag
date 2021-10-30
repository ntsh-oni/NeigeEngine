#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 uv;

layout(set = 1, binding = 0) uniform sampler2D fonts[];

layout(push_constant) uniform TextInfo {
	layout(offset = 96) vec3 color;
	int fontIndex;
} tI;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(fonts[tI.fontIndex], uv).r);
	outColor = vec4(tI.color, 1.0) * sampled;
}