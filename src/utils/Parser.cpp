#include "Parser.h"

GameInfo Parser::parseGame(const std::string& filePath) {
	GameInfo gameInfo = {};

	if (!FileTools::exists(filePath)) {
		NEIGE_WARNING("Could not find " + filePath + " to load game settings.");
		return gameInfo;
	}

	simdjson::ondemand::parser parser;
	auto json = simdjson::padded_string::load(filePath);
	simdjson::ondemand::document document = parser.iterate(json);

	std::string_view name;
	auto error = document["game"]["name"].get(name);
	
	if (!error) {
		gameInfo.name = name;
	}

	int64_t windowWidth;
	error = document["game"]["windowWidth"].get(windowWidth);

	if (!error) {
		gameInfo.windowWidth = static_cast<uint32_t>(windowWidth);
	}

	int64_t windowHeight;
	error = document["game"]["windowHeight"].get(windowHeight);

	if (!error) {
		gameInfo.windowHeight = static_cast<uint32_t>(windowHeight);
	}

	return gameInfo;
}

void Parser::parseScene(const std::string& filePath, ECS& ecs) {
	if (!FileTools::exists(filePath)) {
		NEIGE_ERROR("Could not find " + filePath + " to load the scene.");
	}

	simdjson::ondemand::parser parser;
	auto json = simdjson::padded_string::load(filePath);
	simdjson::ondemand::document document = parser.iterate(json);

	simdjson::error_code error;

	for (simdjson::ondemand::object entity : document["entities"]) {
		Entity ecsEntity = ecs.createEntity();
		for (auto component : entity) {
			std::string_view componentName = std::string_view(component.unescaped_key());
			if (componentName == "camera") {
				Camera camera = {};

				// Position vector
				float position[3];
				simdjson::ondemand::array positionArray;
				error = component.value()["position"].get(positionArray);
				
				if (!error) {
					int index = 0;
					for (auto value : positionArray) {
						position[index] = static_cast<float>(value.get_double());
						index++;
					}

					if (index == 3) {
						camera.position = glm::vec3(position[0], position[1], position[2]);
					}
					else {
						NEIGE_WARNING("Camera position vector is missing " + std::to_string(3 - index) + " values.");
						camera.position = glm::vec3(0.0f, 0.0f, 0.0f);
					}
				}
				else {
					camera.position = glm::vec3(0.0f, 0.0f, 0.0f);
				}

				// To vector
				float to[3];
				simdjson::ondemand::array toArray;
				error = component.value()["to"].get(toArray);

				if (!error) {
					int index = 0;
					for (auto value : toArray) {
						to[index] = static_cast<float>(value.get_double());
						index++;
					}

					if (index == 3) {
						camera.to = glm::vec3(to[0], to[1], to[2]);
					}
					else {
						NEIGE_WARNING("Camera to vector is missing " + std::to_string(3 - index) + " values.");
						camera.to = glm::vec3(1.0f, 0.0f, 0.0f);
					}
				}
				else {
					camera.to = glm::vec3(1.0f, 0.0f, 0.0f);
				}

				// FOV
				double fov;
				error = component.value()["fov"].get(fov);

				if (!error) {
					camera.FOV = static_cast<float>(fov);
				}
				else {
					camera.FOV = 45.0f;
				}

				// Near plane
				double nearPlane;
				error = component.value()["nearPlane"].get(nearPlane);

				if (!error) {
					camera.nearPlane = static_cast<float>(nearPlane);
				}
				else {
					camera.nearPlane = 0.3f;
				}

				// Far plane
				double farPlane;
				error = component.value()["farPlane"].get(farPlane);

				if (!error) {
					camera.farPlane = static_cast<float>(farPlane);
				}
				else {
					camera.farPlane = 200.0f;
				}

				// Envmap
				std::string_view envmapPath;
				error = component.value()["envmapPath"].get(envmapPath);

				if (!error) {
					camera.envmapPath = envmapPath;
				}
				else {
					camera.envmapPath = "";
				}

				ecs.addComponent(ecsEntity, camera);
			}
			else if (componentName == "renderable") {
				Renderable renderable = {};

				// Model
				std::string_view modelPath;
				error = component.value()["modelPath"].get(modelPath);

				if (!error) {
					renderable.modelPath = modelPath;
				}
				else {
					NEIGE_ERROR("Renderable model path is not specified.");
				}

				// Vertex shader
				std::string_view vertexShaderPath;
				error = component.value()["vertexShaderPath"].get(vertexShaderPath);

				if (!error) {
					renderable.vertexShaderPath = vertexShaderPath;
				}
				else {
					NEIGE_ERROR("Renderable vertex shader path is not specified.");
				}

				// Fragment shader
				std::string_view fragmentShaderPath;
				error = component.value()["fragmentShaderPath"].get(fragmentShaderPath);

				if (!error) {
					renderable.fragmentShaderPath = fragmentShaderPath;
				}
				else {
					NEIGE_ERROR("Renderable fragment shader path is not specified.");
				}

				// Tesselation control shader
				std::string_view tesselationControlShaderPath;
				error = component.value()["tesselationControlShaderPath"].get(tesselationControlShaderPath);

				if (!error) {
					renderable.tesselationControlShaderPath = tesselationControlShaderPath;
				}
				else {
					renderable.tesselationControlShaderPath = "";
				}

				// Tesselation evaluation shader
				std::string_view tesselationEvaluationShaderPath;
				error = component.value()["tesselationEvaluationShaderPath"].get(tesselationEvaluationShaderPath);

				if (!error) {
					renderable.tesselationEvaluationShaderPath = tesselationEvaluationShaderPath;
				}
				else {
					renderable.tesselationEvaluationShaderPath = "";
				}

				// Geometry shader
				std::string_view geometryShaderPath;
				error = component.value()["geometryShaderPath"].get(geometryShaderPath);

				if (!error) {
					renderable.geometryShaderPath = geometryShaderPath;
				}
				else {
					renderable.geometryShaderPath = "";
				}

				ecs.addComponent(ecsEntity, renderable);
			}
			else if (componentName == "transform") {
				Transform transform = {};

				// Position vector
				float position[3];
				simdjson::ondemand::array positionArray;
				error = component.value()["position"].get(positionArray);

				if (!error) {
					int index = 0;
					for (auto value : positionArray) {
						position[index] = static_cast<float>(value.get_double());
						index++;
					}

					if (index == 3) {
						transform.position = glm::vec3(position[0], position[1], position[2]);
					}
					else {
						NEIGE_WARNING("Tranform position vector is missing " + std::to_string(3 - index) + " values.");
						transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
					}
				}
				else {
					transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
				}

				// Rotation vector
				float rotation[3];
				simdjson::ondemand::array rotationArray;
				error = component.value()["rotation"].get(rotationArray);

				if (!error) {
					int index = 0;
					for (auto value : rotationArray) {
						rotation[index] = static_cast<float>(value.get_double());
						index++;
					}

					if (index == 3) {
						transform.rotation = glm::vec3(rotation[0], rotation[1], rotation[2]);
					}
					else {
						NEIGE_WARNING("Tranform rotation vector is missing " + std::to_string(3 - index) + " values.");
						transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
					}
				}
				else {
					transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
				}

				// Scale vector
				float scale[3];
				simdjson::ondemand::array scaleArray;
				error = component.value()["scale"].get(scaleArray);

				if (!error) {
					int index = 0;
					for (auto value : scaleArray) {
						scale[index] = static_cast<float>(value.get_double());
						index++;
					}

					if (index == 3) {
						transform.scale = glm::vec3(scale[0], scale[1], scale[2]);
					}
					else {
						NEIGE_WARNING("Tranform rotation vector is missing " + std::to_string(3 - index) + " values.");
						transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
					}
				}
				else {
					transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
				}

				ecs.addComponent(ecsEntity, transform);
			}
			else if (componentName == "light") {
				Light light = {};

				// Type
				std::string_view type;
				error = component.value()["type"].get(type);

				if (!error) {
					if (type == "DIRECTIONAL") {
						light.type = LightType::DIRECTIONAL;
					}
					else if (type == "POINT") {
						light.type = LightType::POINT;
					}
					else if (type == "SPOT") {
						light.type = LightType::SPOT;
					}
					else {
						NEIGE_ERROR("Light type " + std::string(type) + " is undefined (valid options: \"DIRECTIONAL\", \"POINT\" or \"SPOT\").");
					}
				}
				else {
					NEIGE_ERROR("Light type is missing (valid options: \"DIRECTIONAL\", \"POINT\" or \"SPOT\").");
				}

				// Position vector
				if (light.type == LightType::POINT || light.type == LightType::SPOT) {
					float position[3];
					simdjson::ondemand::array positionArray;
					error = component.value()["position"].get(positionArray);

					if (!error) {
						int index = 0;
						for (auto value : positionArray) {
							position[index] = static_cast<float>(value.get_double());
							index++;
						}

						if (index == 3) {
							light.position = glm::vec3(position[0], position[1], position[2]);
						}
						else {
							NEIGE_WARNING("Light position vector is missing " + std::to_string(3 - index) + " values.");
							light.position = glm::vec3(0.0f, 0.0f, 0.0f);
						}
					}
					else {
						light.position = glm::vec3(0.0f, 0.0f, 0.0f);
					}
				}
				else {
					light.position = glm::vec3(0.0f);
				}

				// Direction vector
				if (light.type == LightType::DIRECTIONAL || light.type == LightType::SPOT) {
					float direction[3];
					simdjson::ondemand::array directionArray;
					error = component.value()["direction"].get(directionArray);

					if (!error) {
						int index = 0;
						for (auto value : directionArray) {
							direction[index] = static_cast<float>(value.get_double());
							index++;
						}

						if (index == 3) {
							light.direction = glm::vec3(direction[0], direction[1], direction[2]);
						}
						else {
							NEIGE_WARNING("Light direction vector is missing " + std::to_string(3 - index) + " values.");
							light.direction = glm::vec3(0.0f, 0.0f, 0.0f);
						}
					}
					else {
						light.direction = glm::vec3(0.0f, -1.0f, 0.0f);
					}
				}
				else {
					light.direction = glm::vec3(0.0f);
				}

				// Color vector
				float color[3];
				simdjson::ondemand::array colorArray;
				error = component.value()["color"].get(colorArray);

				if (!error) {
					int index = 0;
					for (auto value : colorArray) {
						color[index] = static_cast<float>(value.get_double());
						index++;
					}

					if (index == 3) {
						light.color = glm::vec3(color[0], color[1], color[2]);
					}
					else {
						NEIGE_WARNING("Light color vector is missing " + std::to_string(3 - index) + " values.");
						light.color = glm::vec3(1.0f, 1.0f, 1.0f);
					}
				}
				else {
					light.color = glm::vec3(1.0f, 1.0f, 1.0f);
				}

				// Cutoffs vector
				if (light.type == LightType::SPOT) {
					float cutoffs[2];
					simdjson::ondemand::array cutoffsArray;
					error = component.value()["cutoffs"].get(cutoffsArray);

					if (!error) {
						int index = 0;
						for (auto value : cutoffsArray) {
							cutoffs[index] = static_cast<float>(value.get_double());
							index++;
						}

						if (index == 2) {
							light.cutoffs = glm::vec2(cutoffs[0], cutoffs[1]);
						}
						else {
							NEIGE_WARNING("Light cutoffs vector is missing " + std::to_string(2 - index) + " values.");
							light.cutoffs = glm::vec2(10.0f, 20.0f);
						}
					}
					else {
						light.cutoffs = glm::vec2(1.0f, 1.0f);
					}
				}
				else {
					light.cutoffs = glm::vec2(0.0f);
				}

				ecs.addComponent(ecsEntity, light);
			}
			else if (componentName == "rigidbody") {
				Rigidbody rigidbody = {};
				
				// Velocity vector
				float velocity[3];
				simdjson::ondemand::array velocityArray;
				error = component.value()["velocity"].get(velocityArray);

				if (!error) {
					int index = 0;
					for (auto value : velocityArray) {
						velocity[index] = static_cast<float>(value.get_double());
						index++;
					}

					if (index == 3) {
						rigidbody.velocity = glm::vec3(velocity[0], velocity[1], velocity[2]);
					}
					else {
						NEIGE_WARNING("Rigidbody velocity vector is missing " + std::to_string(3 - index) + " values.");
						rigidbody.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
					}
				}
				else {
					rigidbody.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
				}

				// Acceleration vector
				float acceleration[3];
				simdjson::ondemand::array accelerationArray;
				error = component.value()["acceleration"].get(accelerationArray);

				if (!error) {
					int index = 0;
					for (auto value : accelerationArray) {
						acceleration[index] = static_cast<float>(value.get_double());
						index++;
					}

					if (index == 3) {
						rigidbody.acceleration = glm::vec3(acceleration[0], acceleration[1], acceleration[2]);
					}
					else {
						NEIGE_WARNING("Rigidbody acceleration vector is missing " + std::to_string(3 - index) + " values.");
						rigidbody.acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
					}
				}
				else {
					rigidbody.acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
				}

				// Affected by gravity
				bool affectedByGravity;
				error = component.value()["affectedByGravity"].get(affectedByGravity);

				if (!error) {
					rigidbody.affectedByGravity = affectedByGravity;
				}
				else {
					rigidbody.affectedByGravity = false;
				}

				ecs.addComponent(ecsEntity, rigidbody);
			}
		}
	}
}
