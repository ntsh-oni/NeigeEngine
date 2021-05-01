#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout(push_constant) uniform PushConstants {
	int diffuseIndex;
	int normalIndex;
	int metallicRoughnessIndex;
	int emissiveIndex;
	int occlusionIndex;
	float alphaCutoff;
} pC;

layout(set = 1, binding = 0) uniform sampler2D colorMapsId[];

layout(location = 0) in vec2 uv;

void main() {
	float opacity = texture(colorMapsId[pC.diffuseIndex], uv).w;
	if (opacity <= pC.alphaCutoff) {
		discard;
	}
}