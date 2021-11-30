#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 uv;

layout(push_constant) uniform RectangleInfo {
	layout(offset = 96) vec3 color;
} rI;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(rI.color, 1.0);
}