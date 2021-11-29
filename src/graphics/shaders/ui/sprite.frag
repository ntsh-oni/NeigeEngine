#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 uv;

layout(set = 1, binding = 0) uniform sampler2D sprites[];

layout(push_constant) uniform SpriteInfo {
	layout(offset = 96) int spriteIndex;
} sI;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(sprites[sI.spriteIndex], uv);
}