#include "SceneMain/SceneMain.hpp"

int main() {
	Log::setFlags(Log::fTimestamp|Log::fAlwaysSave|Log::fStandardOut);
	Game* game = new Game();
	SceneMain* sc = new SceneMain();
	sc->addTo(game);
	game->run();
	delete game;
	return 42;
}
