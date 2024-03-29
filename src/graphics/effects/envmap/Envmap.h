#pragma once
#include "vulkan/vulkan.h"
#include "../../../ecs/components/Camera.h"
#include "../../structs/ShaderStructs.h"
#include "../../structs/RendererStructs.h"
#include "../../commands/CommandPool.h"
#include "../../commands/CommandBuffer.h"
#include "../../pipelines/Viewport.h"
#include "../../pipelines/DescriptorSet.h"
#include "../../pipelines/GraphicsPipeline.h"
#include "../../resources/Buffer.h"
#include "../../resources/Image.h"
#include "../../renderpasses/Framebuffer.h"
#include "../../renderpasses/RenderPass.h"
#include "../../resources/BufferTools.h"
#include "../../resources/ImageTools.h"
#include <numeric>

#define CONVOLVE_WIDTH 32
#define CONVOLVE_HEIGHT 32

#define PREFILTER_WIDTH 512
#define PREFILTER_HEIGHT 512

#define BRDFCONVOLUTION_WIDTH 512
#define BRDFCONVOLUTION_HEIGHT 512

struct Envmap {
	// Unit cube
	Buffer cubeVertexBuffer;
	Buffer cubeIndexBuffer;

	Image defaultSkybox;
	Image envmapImage;
	Image skyboxImage;
	Image diffuseIradianceImage;
	Image prefilterImage;
	Image brdfConvolutionImage;

	// Skybox
	GraphicsPipeline skyboxGraphicsPipeline;
	std::vector<DescriptorSet> skyboxDescriptorSets;

	std::vector<Vertex> cubeVertices = { Vertex { glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f,  1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },

		Vertex { glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },

		Vertex { glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f, -1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f,  1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },

		Vertex { glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f, -1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },

		Vertex { glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f,  1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },

		Vertex { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) },
		Vertex { glm::vec3(1.0f, -1.0f,  1.0f), glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) } };

	void init(std::string filePath, Viewport* fullscreenViewport, RenderPass* opaqueSceneRenderPass);
	void destroy();
	void draw(CommandBuffer* commandBuffer);
	void equiRectangleToCubemap();
	void createDiffuseIradiance();
	void createPrefilter();
	void createBRDFConvolution();
};