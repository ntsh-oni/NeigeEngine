#version 460

layout(set = 0, binding = 1) uniform samplerCube skybox;

layout(location = 0) in vec3 uv;

layout(location = 0) out vec4 sceneColor;

void main() {
	vec3 skyboxSample = vec3(texture(skybox, uv));
	
	sceneColor = vec4(skyboxSample, 1.0);
}