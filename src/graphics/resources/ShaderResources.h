#pragma once
#include "../../utils/structs/ShaderStructs.h"
#include "../models/Model.h"
#include "../pipelines/Shader.h"
#include "../effects/bloom/Bloom.h"
#include "../effects/depthprepass/DepthPrepass.h"
#include "../effects/envmap/Envmap.h"
#include "../effects/fxaa/FXAA.h"
#include "../effects/shadowmapping/Shadow.h"
#include "../effects/ssao/SSAO.h"
#include "../../ecs/ECS.h"
#include "Image.h"
#include <string>
#include <unordered_map>

inline std::unordered_map<std::string, Model> models;
inline std::unordered_map<std::string, Image> textures;
inline std::vector<Material> materials;
inline std::unordered_map<std::string, Shader> shaders;
inline Entity camera;
inline std::vector<Buffer> cameraBuffers;
inline std::set<Entity> lights;
inline std::vector<Buffer> lightingBuffers;
inline std::vector<Buffer> timeBuffers;
inline Image sceneImage;
inline Image blendAccumulationImage;
inline Image blendRevealageImage;
inline Image postProcessImage;
inline Bloom bloom;
inline DepthPrepass depthPrepass;
inline Envmap envmap;
inline FXAA fxaa;
inline Shadow shadow;
inline SSAO ssao;