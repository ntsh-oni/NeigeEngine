#version 450

layout(push_constant) uniform AlphaCutoff {
	float alphaCutoff;
} alphaCutoff;

layout(set = 1, binding = 0) uniform sampler2D colorMap;

layout(location = 0) in vec2 uv;

void main() {
	float opacity = texture(colorMap, uv).w;
	if (opacity <= alphaCutoff.alphaCutoff) {
		discard;
	}
}