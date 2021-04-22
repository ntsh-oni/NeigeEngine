#include "src/Game.h"
#include "src/utils/Parser.h"
#include <random>

ECS ecs;

int main(void) {
	ecs.init();

	GameInfo gameInfo = Parser::parseGame("../game.json");

	Game g;
	g.applicationName = gameInfo.name;

	window.extent.width = gameInfo.windowWidth;
	window.extent.height = gameInfo.windowHeight;

	g.enableBloom = gameInfo.enableBloom;
	g.enableSSAO = gameInfo.enableSSAO;
	g.enableFXAA = gameInfo.enableFXAA;

	g.init();

	Parser::parseScene("../scene0.json", ecs);

	g.launch();
}