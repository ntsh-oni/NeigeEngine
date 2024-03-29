#pragma once
#include "../structs/ShaderStructs.h"
#include "../models/Model.h"
#include "../pipelines/Shader.h"
#include "../effects/atmosphere/Atmosphere.h"
#include "../effects/bloom/Bloom.h"
#include "../effects/depthprepass/DepthPrepass.h"
#include "../effects/envmap/Envmap.h"
#include "../effects/frustumculling/FrustumCulling.h"
#include "../effects/fxaa/FXAA.h"
#include "../effects/shadowmapping/Shadow.h"
#include "../effects/ssao/SSAO.h"
#include "../effects/ui/UI.h"
#include "../../ecs/ECS.h"
#include "Buffer.h"
#include "Image.h"
#include <string>
#include <unordered_map>

inline std::unordered_map<std::string, Model> models;
inline std::vector<Texture> textures;
inline std::vector<Material> materials;
inline DescriptorPool materialsDescriptorPool;
inline VkDescriptorSetLayout materialsDescriptorSetLayout = VK_NULL_HANDLE;
inline std::vector<DescriptorSet> materialsDescriptorSets;
inline std::vector<bool> materialDescriptorSetUpToDate;
inline DescriptorPool perDrawDescriptorPool;
inline VkDescriptorSetLayout perDrawDescriptorSetLayout = VK_NULL_HANDLE;
inline std::unordered_map<std::string, Shader> shaders;
inline Entity mainCamera;
inline std::set<Entity>* cameras;
inline std::vector<Buffer> cameraBuffers;
inline std::vector<Buffer> frustumBuffers;
inline float mainDirectionalLightDirection[3] = { 0.0f, 0.0f, 0.0f };
inline std::set<Entity>* lights;
inline std::vector<Buffer> lightingBuffers;
inline std::vector<Buffer> timeBuffers;
inline Image sceneImage;
inline Image blendAccumulationImage;
inline Image blendRevealageImage;
inline Image postProcessImage;
inline Atmosphere atmosphere;
inline Bloom bloom;
inline DepthPrepass depthPrepass;
inline Envmap envmap;
inline FrustumCulling frustumCulling;
inline FXAA fxaa;
inline Shadow shadow;
inline SSAO ssao;
inline UI ui;