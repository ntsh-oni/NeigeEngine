#pragma once
#include "../../external/simdjson/singleheader/simdjson.h"
#include "../ecs/ECS.h"
#include "../ecs/components/Camera.h"
#include "../ecs/components/Light.h"
#include "../ecs/components/Renderable.h"
#include "../ecs/components/Rigidbody.h"
#include "../ecs/components/Script.h"
#include "../ecs/components/Transform.h"
#include "resources/FileTools.h"
#include "../Game.h"
#include "../Scene.h"

struct Parser {
	static GameInfo parseGameInfo(const std::string& filePath);
	static Scene parseScene(const std::string& filePath, ECS& ecs);
};