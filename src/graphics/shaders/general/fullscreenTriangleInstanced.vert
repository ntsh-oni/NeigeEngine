#version 460
#extension GL_ARB_shader_viewport_layer_array : enable

layout(location = 0) out vec2 outUv;
layout(location = 1) out flat int instanceID;

void main() {
	outUv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	instanceID = gl_InstanceIndex;
	gl_Layer = instanceID;
	gl_Position = vec4(outUv * 2.0 + -1.0, 0.0, 1.0);
}