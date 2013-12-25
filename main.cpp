#include "SceneMain/SceneMain.hpp"

int main() {
	WINDOW_TITLE = "Deferred Test";
	Game* game = new Game();
    SceneMain* sc = new SceneMain();
	sc->addTo(game);
	game->run();
	delete game;
    return 42;
}
