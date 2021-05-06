#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 1, binding = 0) uniform sampler2D textures[];
layout(set = 1, binding = 1) restrict readonly buffer Materials {
	int diffuseIndex;
	int normalIndex;
	int metallicRoughnessIndex;
	int emissiveIndex;
	int occlusionIndex;
} materials[];

layout(push_constant) uniform PushConstants {
	int materialIndex;
	float alphaCutoff;
} pC;

layout(location = 0) in vec2 uv;

void main() {
	float opacity = texture(textures[materials[pC.materialIndex].diffuseIndex], uv).w;
	if (opacity <= pC.alphaCutoff) {
		discard;
	}
}