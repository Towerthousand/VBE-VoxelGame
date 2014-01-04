#include "SceneMain/SceneMain.hpp"

int main() {
	WINDOW_TITLE = "Deferred Test";
	ZNEAR = 0.01f;
	ZFAR = 1000.0f;
	Game* game = new Game();
    SceneMain* sc = new SceneMain();
	sc->addTo(game);
	game->run();
	delete game;
    return 42;
}
