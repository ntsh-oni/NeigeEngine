
# NeigeEngine
![NeigeEngine](https://i.imgur.com/JcU2ZnS.jpg)

## Dependencies
### To install
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
### Provided as submodules
- [cgltf](https://github.com/jkuhlmann/cgltf)
- [GLFW](https://www.glfw.org/)
- [GLM](https://github.com/g-truc/glm)
- [Glslang](https://github.com/KhronosGroup/glslang)
- [simdjson](https://github.com/simdjson/simdjson)
- [SPIRV-Reflect](https://github.com/KhronosGroup/SPIRV-Reflect)
- [stb](https://github.com/nothings/stb)
## Download
```txt
$ git clone --recurse-submodules https://github.com/ntsh-oni/NeigeEngine.git
```
## Build
```txt
$ mkdir build
$ cd build
$ cmake ..
$ make
```
## Features
### General Features
- [x] Entity component system
- [x] Keyboard and mouse input
- [x] glTF import
- [x] JSON scene format
### Graphics Features
- [x] Physically Based Rendering (PBR) metallic-roughness
- [x] Image-Based Lighting (IBL)
- [x] Shadow mapping
- [x] Fast Approximate Anti-Aliasing (FXAA)
- [x] Pure Depth Screen Space Ambient Occlusion (SSAO)
- [x] Bloom
- [x] Weighted Blended Order-Independent Transparency (WBOIT)
- [x] Alpha cutoff
- [x] Frustum culling
- [x] Realtime shaders reloading
## Screenshots
![Amazon Lumberyard Bistro](https://i.imgur.com/WVOoWP8.png)
<sub>Amazon Lumberyard Bistro</sub>


![Damaged Helmet](https://i.imgur.com/rXocPqI.png)
<sub>Damaged Helmet</sub>
