#include "SceneMain/SceneMain.hpp"

int main() {
	Environment::setup().windowTitle = ":3";
	Environment::setup().windowHeight = 1080;
	Environment::setup().windowWidth = 1920;
	Environment::setup().windowFlags = Screen::WINDOW_SHOWN | Screen::WINDOW_OPENGL;
	Environment::setup().mouseGrab = true;
	Environment::setup().mouseRelativeMode = true;
	Game* game = new Game();
	SceneMain* sc = new SceneMain();
	sc->addTo(game);
	game->run();
	delete game;
	return 0;
}
