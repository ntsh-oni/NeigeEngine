cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
SET(NAME NeigeEngine)
PROJECT(${NAME} VERSION 0.0.1 LANGUAGES C CXX)

SET(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
SET(ALSOFT_EMBED_HRTF_DATA OFF CACHE BOOL "" FORCE)
SET(ALSOFT_INSTALL_AMBDEC_PRESETS OFF CACHE BOOL "" FORCE)
SET(ALSOFT_INSTALL_CONFIG OFF CACHE BOOL "" FORCE)
SET(ALSOFT_INSTALL_EXAMPLES OFF CACHE BOOL "" FORCE)
SET(ALSOFT_INSTALL_HRTF_DATA OFF CACHE BOOL "" FORCE)
SET(ALSOFT_INSTALL_UTILS OFF CACHE BOOL "" FORCE)
SET(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
SET(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
SET(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
SET(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

add_subdirectory(external/glslang)

add_subdirectory(external/luabuild)

add_subdirectory(external/openal-soft)

IF (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
	message(STATUS "Looking for Vulkan...")
	find_package(Vulkan)
ENDIF()

IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
	message(STATUS ${Vulkan_LIBRARY})
ENDIF()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch-enum")
ENDIF()

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(GAME_SOURCES src/Game.cpp)
SET(GAME_HEADERS src/Game.h src/Scene.h)

SET(AUDIO_RESOURCES_SOURCES src/audio/resources/AudioTools.cpp src/audio/resources/Sound.cpp)
SET(AUDIO_RESOURCES_HEADERS src/audio/resources/AudioChecks.h src/audio/resources/AudioResources.h src/audio/resources/AudioTools.h src/audio/resources/Sound.h)

SET(AUDIO_SOURCES src/audio/Audio.cpp ${AUDIO_RESOURCES_SOURCES})
SET(AUDIO_HEADERS src/audio/Audio.h ${AUDIO_RESOURCES_HEADERS})

SET(GRAPHICS_COMMANDS_SOURCES src/graphics/commands/CommandBuffer.cpp src/graphics/commands/CommandPool.cpp)
SET(GRAPHICS_COMMANDS_HEADERS src/graphics/commands/CommandBuffer.h src/graphics/commands/CommandPool.h)
SET(GRAPHICS_DEVICES_SOURCES src/graphics/devices/LogicalDevice.cpp src/graphics/devices/PhysicalDevice.cpp src/graphics/devices/PhysicalDevicePicker.cpp)
SET(GRAPHICS_DEVICES_HEADERS src/graphics/devices/LogicalDevice.h src/graphics/devices/PhysicalDevice.h src/graphics/devices/PhysicalDevicePicker.h)
SET(GRAPHICS_EFFECTS_SOURCES src/graphics/effects/atmosphere/Atmosphere.cpp src/graphics/effects/bloom/Bloom.cpp src/graphics/effects/depthprepass/DepthPrepass.cpp src/graphics/effects/envmap/Envmap.cpp src/graphics/effects/frustumculling/FrustumCulling.cpp src/graphics/effects/fxaa/FXAA.cpp src/graphics/effects/shadowmapping/Shadow.cpp src/graphics/effects/ssao/SSAO.cpp src/graphics/effects/ui/UI.cpp)
SET(GRAPHICS_EFFECTS_HEADERS src/graphics/effects/atmosphere/Atmosphere.h src/graphics/effects/bloom/Bloom.h src/graphics/effects/depthprepass/DepthPrepass.h src/graphics/effects/envmap/Envmap.h src/graphics/effects/frustumculling/FrustumCulling.h src/graphics/effects/fxaa/FXAA.h src/graphics/effects/shadowmapping/Shadow.h src/graphics/effects/ssao/SSAO.h src/graphics/effects/ui/UI.h)
SET(GRAPHICS_INSTANCE_SOURCES src/graphics/instance/Instance.cpp)
SET(GRAPHICS_INSTANCE_HEADERS src/graphics/instance/Instance.h)
SET(GRAPHICS_MODELS_SOURCES src/graphics/models/Model.cpp)
SET(GRAPHICS_MODELS_HEADERS src/graphics/models/Model.h)
SET(GRAPHICS_PIPELINES_SOURCES src/graphics/pipelines/ComputePipeline.cpp src/graphics/pipelines/DescriptorPool.cpp src/graphics/pipelines/DescriptorSet.cpp src/graphics/pipelines/GraphicsPipeline.cpp src/graphics/pipelines/Shader.cpp src/graphics/pipelines/Viewport.cpp)
SET(GRAPHICS_PIPELINES_HEADERS src/graphics/pipelines/ComputePipeline.h src/graphics/pipelines/DescriptorPool.h src/graphics/pipelines/DescriptorSet.h src/graphics/pipelines/GraphicsPipeline.h src/graphics/pipelines/Shader.h src/graphics/pipelines/Viewport.h)
SET(GRAPHICS_RENDERPASSES_SOURCES src/graphics/renderpasses/Framebuffer.cpp src/graphics/renderpasses/RenderPass.cpp src/graphics/renderpasses/RenderPassAttachment.cpp src/graphics/renderpasses/Swapchain.cpp)
SET(GRAPHICS_RENDERPASSES_HEADERS src/graphics/renderpasses/Framebuffer.h src/graphics/renderpasses/RenderPass.h src/graphics/renderpasses/RenderPassAttachment.h src/graphics/renderpasses/Swapchain.h)
SET(GRAPHICS_RESOURCES_SOURCES src/graphics/resources/Buffer.cpp src/graphics/resources/BufferTools.cpp src/graphics/resources/Image.cpp src/graphics/resources/ImageTools.cpp src/graphics/resources/ModelLoader.cpp)
SET(GRAPHICS_RESOURCES_HEADERS src/graphics/resources/Buffer.h src/graphics/resources/BufferTools.h src/graphics/resources/Image.h src/graphics/resources/ImageTools.h src/graphics/resources/ModelLoader.h src/graphics/resources/RendererResources.h src/graphics/resources/Samplers.h src/graphics/resources/ShaderResources.h src/graphics/resources/UIResources.h)
SET(GRAPHICS_STRUCTS_HEADERS src/graphics/structs/ModelStructs.h src/graphics/structs/RendererStructs.h src/graphics/structs/ShaderStructs.h src/graphics/structs/UIStructs.h)
SET(GRAPHICS_SYNC_SOURCES src/graphics/sync/Fence.cpp src/graphics/sync/Semaphore.cpp)
SET(GRAPHICS_SYNC_HEADERS src/graphics/sync/Fence.h src/graphics/sync/Semaphore.h)

SET(GRAPHICS_SOURCES src/graphics/Renderer.cpp ${GRAPHICS_COMMANDS_SOURCES} ${GRAPHICS_DEVICES_SOURCES} ${GRAPHICS_EFFECTS_SOURCES} ${GRAPHICS_INSTANCE_SOURCES} ${GRAPHICS_MODELS_SOURCES} ${GRAPHICS_PIPELINES_SOURCES} ${GRAPHICS_RENDERPASSES_SOURCES} ${GRAPHICS_RESOURCES_SOURCES} ${GRAPHICS_SYNC_SOURCES})
SET(GRAPHICS_HEADERS src/graphics/Renderer.h ${GRAPHICS_COMMANDS_HEADERS} ${GRAPHICS_DEVICES_HEADERS} ${GRAPHICS_EFFECTS_HEADERS} ${GRAPHICS_INSTANCE_HEADERS} ${GRAPHICS_MODELS_HEADERS} ${GRAPHICS_PIPELINES_HEADERS} ${GRAPHICS_RENDERPASSES_HEADERS} ${GRAPHICS_RESOURCES_HEADERS} ${GRAPHICS_STRUCTS_HEADERS} ${GRAPHICS_SYNC_HEADERS})

SET(PHYSICS_SOURCES src/physics/AABB.cpp src/physics/Frustum.cpp src/physics/Physics.cpp)
SET(PHYSICS_HEADERS src/physics/AABB.h src/physics/Frustum.h src/physics/Physics.h)

SET(SCRIPTING_RESOURCES_HEADERS src/scripting/resources/ScriptingResources.h)
SET(SCRIPTING_SOURCES src/scripting/AudioScripting.cpp src/scripting/CameraScripting.cpp src/scripting/DebugScripting.cpp src/scripting/EntityScripting.cpp src/scripting/InputScripting.cpp src/scripting/PhysicsScripting.cpp src/scripting/Scripting.cpp src/scripting/TimeScripting.cpp src/scripting/UIScripting.cpp src/scripting/VectorScripting.cpp src/scripting/WindowScripting.cpp)
SET(SCRIPTING_HEADERS ${SCRIPTING_RESOURCES_HEADERS} src/scripting/AudioScripting.h src/scripting/CameraScripting.h src/scripting/DebugScripting.h src/scripting/EntityScripting.h src/scripting/InputScripting.h src/scripting/PhysicsScripting.h src/scripting/Scripting.h src/scripting/TimeScripting.h src/scripting/UIScripting.h src/scripting/VectorScripting.h src/scripting/WindowScripting.h)

SET(UTILS_MEMORYALLOCATOR_SOURCES src/utils/memoryallocator/MemoryAllocator.cpp)
SET(UTILS_MEMORYALLOCATOR_HEADERS src/utils/memoryallocator/MemoryAllocator.h)
SET(UTILS_RESOURCES_SOURCES src/utils/resources/FileTools.cpp)
SET(UTILS_RESOURCES_HEADERS src/utils/resources/FileTools.h)
SET(UTILS_SOURCES src/utils/NeigeVKTranslate.cpp src/utils/Parser.cpp ${UTILS_MEMORYALLOCATOR_SOURCES} ${UTILS_RESOURCES_SOURCES})
SET(UTILS_HEADERS src/utils/NeigeDefines.h src/utils/NeigeVKTranslate.h src/utils/Parser.h ${UTILS_MEMORYALLOCATOR_HEADERS} ${UTILS_RESOURCES_HEADERS})

SET(WINDOW_SOURCES src/window/Surface.cpp src/window/Window.cpp)
SET(WINDOW_HEADERS src/window/Surface.h src/window/WindowResources.h src/window/Window.h)

SET(INPUTS_SOURCES src/inputs/KeyboardInputs.cpp src/inputs/MouseInputs.cpp)
SET(INPUTS_HEADERS src/inputs/Inputs.h src/inputs/KeyboardInputs.h src/inputs/MouseInputs.h)

SET(ECS_COMPONENTS_HEADERS src/ecs/components/Camera.h src/ecs/components/Light.h src/ecs/components/Renderable.h src/ecs/components/Rigidbody.h src/ecs/components/Script.h src/ecs/components/Transform.h)
SET(ECS_SYSTEMS_SOURCES src/ecs/systems/CameraSystem.cpp src/ecs/systems/Lighting.cpp)
SET(ECS_SYSTEMS_HEADERS src/ecs/systems/CameraSystem.h src/ecs/systems/Lighting.h)
SET(ECS_SOURCES ${ECS_SYSTEMS_SOURCES})
SET(ECS_HEADERS src/ecs/ECS.h ${ECS_COMPONENTS_HEADERS} ${ECS_SYSTEMS_HEADERS})

SET(EXTERNAL_SOURCES external/spirv-reflect/spirv_reflect.c)
SET(EXTERNAL_HEADERS external/json/single_include/nlohmann/json.hpp)

SET(SOURCES ${GAME_SOURCES} ${AUDIO_SOURCES} ${GRAPHICS_SOURCES} ${PHYSICS_SOURCES} ${SCRIPTING_SOURCES} ${UTILS_SOURCES} ${WINDOW_SOURCES} ${INPUTS_SOURCES} ${ECS_SOURCES} ${EXTERNAL_SOURCES})
SET(HEADERS ${GAME_HEADERS} ${AUDIO_HEADERS} ${GRAPHICS_HEADERS} ${PHYSICS_HEADERS} ${SCRIPTING_HEADERS} ${UTILS_HEADERS} ${WINDOW_HEADERS} ${INPUTS_HEADERS} ${ECS_HEADERS} ${EXTERNAL_HEADERS})

add_executable(${PROJECT_NAME} main.cpp ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC external/cgltf)
target_include_directories(${PROJECT_NAME} PUBLIC external/glm)
target_include_directories(${PROJECT_NAME} PUBLIC external/lua)
target_include_directories(${PROJECT_NAME} PUBLIC external/spirv-reflect)
target_include_directories(${PROJECT_NAME} PUBLIC external/stb)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw glslang lua OpenAL SPIRV Vulkan::Vulkan)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:OpenAL> ${PROJECT_BINARY_DIR}
COMMENT "Copy OpenAL32 to project binary folder.")