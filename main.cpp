#include "src/Game.h"
#include "src/utils/Parser.h"
#include <random>

ECS ecs;

int main(void) {
	ecs.init();

	Game g;
	g.info = Parser::parseGame("../game.json");

	window.extent.width = g.info.windowWidth;
	window.extent.height = g.info.windowHeight;

	g.init();

	Parser::parseScene("../scene0.json", ecs);

	g.launch();
}