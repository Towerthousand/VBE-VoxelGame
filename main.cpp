#include "SceneMain/SceneMain.hpp"

int main() {
	Log::setFlags(Log::Timestamp|Log::AlwaysSave|Log::StandardOut);

	Environment::startUp();
	Environment::getScreen()->initWindow(":3", 500, 500, Screen::WINDOW_SHOWN | Screen::WINDOW_OPENGL | Screen::WINDOW_BORDERLESS);
	Environment::getMouse()->hideCursor();
	Environment::getMouse()->setGrab(true);
	Game* game = new Game();
	SceneMain* sc = new SceneMain();
	sc->addTo(game);
	game->run();
	delete game;

	Environment::shutDown();

	return 0;
}
