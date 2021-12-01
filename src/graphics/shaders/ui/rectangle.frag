#version 460

layout(location = 0) in vec2 uv;

layout(push_constant) uniform RectangleInfo {
	layout(offset = 96) vec4 color;
} rI;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = rI.color;
}