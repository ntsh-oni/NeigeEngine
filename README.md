# NeigeEngine
<img src="https://i.imgur.com/I5J92Cv.png" alt="NeigeEngine" width="250"/>

## Dependencies
### To install
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
### Provided as submodules
- [cgltf](https://github.com/jkuhlmann/cgltf)
- [GLFW](https://www.glfw.org/)
- [GLM](https://github.com/g-truc/glm)
- [Glslang](https://github.com/KhronosGroup/glslang)
- [lua](https://github.com/lua/lua)
- [OpenAL Soft](https://openal-soft.org/)
- [simdjson](https://github.com/simdjson/simdjson)
- [SPIRV-Reflect](https://github.com/KhronosGroup/SPIRV-Reflect)
- [stb](https://github.com/nothings/stb)
## Download
```txt
git clone --recurse-submodules https://github.com/ntsh-oni/NeigeEngine.git
```
## Build
```txt
mkdir build
cd build
cmake ..
make
```
## Features
### General Features
- Entity component system
- Keyboard and mouse input
- glTF import
- [Lua scripting](docs/scripting/index.md)
- Wav and Ogg Vorbis audio support
- JSON scene format
### Graphics Features
- Physically Based Rendering (PBR) metallic-roughness
- Image-Based Lighting (IBL)
- Atmosphere
- Shadow mapping
- Fast Approximate Anti-Aliasing (FXAA)
- Pure Depth Screen Space Ambient Occlusion (SSAO)
- Bloom
- Weighted Blended Order-Independent Transparency (WBOIT)
- Alpha cutoff
- GPU Frustum culling
- Realtime shaders reloading
## Screenshots
![Amazon Lumberyard Bistro](https://i.imgur.com/uq0IOq4.png)
<sub>Amazon Lumberyard Bistro</sub>


![Damaged Helmet](https://i.imgur.com/lQcuwdv.png)
<sub>Damaged Helmet</sub>
