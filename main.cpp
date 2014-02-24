#include "SceneMain/SceneMain.hpp"

int main() {
	Log::setFlags(Log::Timestamp|Log::AlwaysSave|Log::StandardOut);
	Environment::setup().windowTitle = ":3";
	Environment::setup().windowHeight = 300;
	Environment::setup().windowWidth = 300;
	Environment::setup().windowFlags = Screen::WINDOW_SHOWN | Screen::WINDOW_OPENGL | Screen::WINDOW_FULLSCREEN;
	Game* game = new Game();
	SceneMain* sc = new SceneMain();
	sc->addTo(game);
	game->run();
	delete game;
	return 0;
}
