#include "../../external/simdjson/singleheader/simdjson.h"
#include "../ecs/ECS.h"
#include "../ecs/components/Transform.h"
#include "../ecs/components/Camera.h"
#include "../ecs/components/Renderable.h"
#include "../ecs/components/Light.h"
#include "../ecs/components/Rigidbody.h"
#include "resources/FileTools.h"
#include "../Game.h"

struct Parser {
	static GameInfo parseGame(const std::string& filePath);
	static void parseScene(const std::string& filePath, ECS& ecs);
};