#include "src/Game.h"

ECS ecs;

int main(void) {
	ecs.init();

	Game g;
	g.init();
	g.launch();
}