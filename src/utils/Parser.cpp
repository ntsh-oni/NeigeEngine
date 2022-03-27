#include "Parser.h"
#include "../../external/json/single_include/nlohmann/json.hpp"
#include <fstream>

GameInfo Parser::parseGameInfo(const std::string& filePath) {
	GameInfo gameInfo = {};

	if (!FileTools::exists(filePath)) {
		NEIGE_WARNING("Could not find " + filePath + " to load game settings.");
		return gameInfo;
	}

	std::ifstream file(filePath);
	nlohmann::json j;
	file >> j;

	// Game
	if (j.contains("game")) {
		nlohmann::json gameJson = j["game"];

		// Game name
		if (gameJson.contains("name")) {
			gameInfo.name = gameJson["name"];
		}

		// Window width
		if (gameJson.contains("windowWidth")) {
			gameInfo.windowWidth = gameJson["windowWidth"];
		}

		// Window height
		if (gameJson.contains("windowHeight")) {
			gameInfo.windowHeight = gameJson["windowHeight"];
		}
	}

	// Graphics settings
	if (j.contains("graphics")) {
		nlohmann::json graphicsJson = j["graphics"];

		// Bloom
		if (graphicsJson.contains("bloom")) {
			nlohmann::json bloomJson = graphicsJson["bloom"];

			if (bloomJson.contains("enable")) {
				gameInfo.enableBloom = bloomJson["enable"];
			}

			if (bloomJson.contains("downscale")) {
				gameInfo.bloomDownscale = bloomJson["downscale"];
			}

			if (bloomJson.contains("threshold")) {
				gameInfo.bloomThreshold = bloomJson["threshold"];
			}

			if (bloomJson.contains("blurBigKernel")) {
				gameInfo.bloomBlurBigKernel = bloomJson["blurBigKernel"];
			}
		}

		// SSAO
		if (graphicsJson.contains("ssao")) {
			nlohmann::json ssaoJson = graphicsJson["ssao"];

			if (ssaoJson.contains("enable")) {
				gameInfo.enableSSAO = ssaoJson["enable"];
			}

			if (ssaoJson.contains("downscale")) {
				gameInfo.ssaoDownscale = ssaoJson["downscale"];
			}
		}

		// FXAA
		if (graphicsJson.contains("fxaa")) {
			nlohmann::json fxaaJson = graphicsJson["fxaa"];

			if (fxaaJson.contains("enable")) {
				gameInfo.enableFXAA = fxaaJson["enable"];
			}
		}
	}

	return gameInfo;
}

Scene Parser::parseScene(const std::string& filePath, ECS& ecs) {
	Scene scene = {};

	if (!FileTools::exists(filePath)) {
		NEIGE_ERROR("Could not find " + filePath + " to load the scene.");
		return scene;
	}

	std::ifstream file(filePath);
	nlohmann::json j;
	file >> j;

	// Skybox
	if (j.contains("skybox")) {
		nlohmann::json skyboxJson = j["skybox"];

		// Type
		if (skyboxJson.contains("type")) {
			std::string skyboxType = skyboxJson["type"];

			if (skyboxType == "ENVMAP") {
				scene.skyboxType = SkyboxType::ENVMAP;
			}
			else if (skyboxType == "ATMOSPHERE") {
				scene.skyboxType = SkyboxType::ATMOSPHERE;
			}
			else {
				NEIGE_WARNING("Skybox type \"" + std::string(skyboxType) + "\" is undefined (valid options: \"ATMOSPHERE\" or \"ENVMAP\".");
				scene.skyboxType = SkyboxType::ATMOSPHERE;
			}
		}

		std::string envmapPath = j["skybox"]["envmapPath"];
		scene.envmapPath = envmapPath;
	}

	for (auto entity : j["entities"]) {
		Entity ecsEntity = ecs.createEntity();
		
		if (entity.contains("camera")) {
			nlohmann::json cameraJson = entity["camera"];

			Camera camera = {};

			// FOV
			if (cameraJson.contains("fov")) {
				camera.component.FOV = cameraJson["fov"];
			}

			// Near plane
			if (cameraJson.contains("nearPlane")) {
				camera.component.nearPlane = cameraJson["nearPlane"];
			}

			// Far plane
			if (cameraJson.contains("farPlane")) {
				camera.component.farPlane = cameraJson["farPlane"];
			}

			ecs.addComponent(ecsEntity, camera);
		}
		
		if (entity.contains("light")) {
			nlohmann::json lightJson = entity["light"];

			Light light = {};

			// Type
			if (lightJson.contains("light")) {
				std::string type = lightJson["type"];

				if (type == "DIRECTIONAL") {
					light.component.type = LightType::DIRECTIONAL;
				}
				else if (type == "POINT") {
					light.component.type = LightType::POINT;
				}
				else if (type == "SPOT") {
					light.component.type = LightType::SPOT;
				}
				else {
					NEIGE_ERROR("Light type " + std::string(type) + " is undefined (valid options: \"DIRECTIONAL\", \"POINT\" or \"SPOT\").");
				}
			}

			// Color vector
			if (lightJson.contains("color")) {
				float color[3];
				color[0] = lightJson["color"][0];
				color[1] = lightJson["color"][1];
				color[2] = lightJson["color"][2];
				light.component.color = glm::vec3(color[0], color[1], color[2]);
			}

			// Cutoffs vector
			if (lightJson.contains("cutoffs")) {
				if (light.component.type == LightType::SPOT) {
					float cutoffs[2];
					cutoffs[0] = lightJson["cutoffs"][0];
					cutoffs[1] = lightJson["cutoffs"][1];
					light.component.cutoffs = glm::vec2(cutoffs[0], cutoffs[1]);

				}
			}

			ecs.addComponent(ecsEntity, light);
		}
		
		if (entity.contains("renderable")) {
			nlohmann::json renderableJson = entity["renderable"];

			Renderable renderable = {};

			// Model
			if (renderableJson.contains("modelPath")) {
				renderable.component.modelPath = renderableJson["modelPath"];
			}

			// Vertex shader
			if (renderableJson.contains("vertexShaderPath")) {
				renderable.component.vertexShaderPath = renderableJson["vertexShaderPath"];
			}

			// Fragment shader
			if (renderableJson.contains("fragmentShaderPath")) {
				renderable.component.fragmentShaderPath = renderableJson["fragmentShaderPath"];
			}

			// Tesselation control shader
			if (renderableJson.contains("tesselationControlShaderPath")) {
				renderable.component.tesselationControlShaderPath = renderableJson["tesselationControlShaderPath"];
			}

			// Tesselation evaluation shader
			if (renderableJson.contains("tesselationEvaluationShaderPath")) {
				renderable.component.tesselationEvaluationShaderPath = renderableJson["tesselationEvaluationShaderPath"];
			}

			// Geometry shader
			if (renderableJson.contains("geometryShaderPath")) {
				renderable.component.geometryShaderPath = renderableJson["geometryShaderPath"];
			}

			ecs.addComponent(ecsEntity, renderable);
		}
		
		if (entity.contains("rigidbody")) {
			nlohmann::json rigidbodyJson = entity["rigidbody"];

			Rigidbody rigidbody = {};

			// Affected by gravity
			if (rigidbodyJson.contains("affectedByGravity")) {
				rigidbody.component.affectedByGravity = rigidbodyJson["affectedByGravity"];
			}

			ecs.addComponent(ecsEntity, rigidbody);
		}
		
		if (entity.contains("script")) {
			nlohmann::json scriptJson = entity["script"];

			Script script = {};

			if (scriptJson.contains("scriptPath")) {
				script.component.scriptPath = scriptJson["scriptPath"];
			}

			ecs.addComponent(ecsEntity, script);
		}
		
		if (entity.contains("transform")) {
			nlohmann::json transformJson = entity["transform"];

			Transform transform = {};

			// Position vector
			if (transformJson.contains("position")) {
				float position[3];
				position[0] = transformJson["position"][0];
				position[1] = transformJson["position"][1];
				position[2] = transformJson["position"][2];
				transform.component.position = glm::vec3(position[0], position[1], position[2]);
			}

			// Rotation vector
			if (transformJson.contains("rotation")) {
				float rotation[3];
				rotation[0] = transformJson["rotation"][0];
				rotation[1] = transformJson["rotation"][1];
				rotation[2] = transformJson["rotation"][2];
				transform.component.rotation = glm::vec3(rotation[0], rotation[1], rotation[2]);
			}

			// Scale vector
			if (transformJson.contains("scale")) {
				float scale[3];
				scale[0] = transformJson["scale"][0];
				scale[1] = transformJson["scale"][1];
				scale[2] = transformJson["scale"][2];
				transform.component.scale = glm::vec3(scale[0], scale[1], scale[2]);
			}

			ecs.addComponent(ecsEntity, transform);
		}
		else {
			// Obligatory Transform component
			ecs.addComponent(ecsEntity, Transform{
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(1.0f, 1.0f, 1.0f)
				});
		}
	}

	return scene;
}
