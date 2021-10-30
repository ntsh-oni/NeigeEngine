#version 460
#extension GL_EXT_nonuniform_qualifier : enable

struct PerDrawMaterial {
	int materialIndex;
	float alphaCutoff;
};

layout(set = 1, binding = 0) uniform sampler2D textures[];
layout(set = 1, binding = 1) restrict readonly buffer Materials {
	int diffuseIndex;
	int normalIndex;
	int metallicRoughnessIndex;
	int emissiveIndex;
	int occlusionIndex;
} materials[];

layout(set = 2, binding = 0) restrict readonly buffer PerDraw {
	PerDrawMaterial perDrawMaterial[];
} perDraw;

layout(location = 0) in vec2 uv;
layout(location = 1) in flat int drawIndex;

void main() {
	float opacity = texture(textures[materials[perDraw.perDrawMaterial[drawIndex].materialIndex].diffuseIndex], uv).w;
	if (opacity <= perDraw.perDrawMaterial[drawIndex].alphaCutoff) {
		discard;
	}
}