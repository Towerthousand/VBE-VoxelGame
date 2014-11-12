#include "SceneMain/SceneMain.hpp"

int main() {
	ContextSettings s;
	s.versionMajor = 4;
	s.versionMinor = 3;
	Window w(Window::getFullscreenModes()[0], s);
	w.setTitle("VoxelGame");
	Mouse::setGrab(false);
	Mouse::setRelativeMode(true);
	Game* game = new Game();
	SceneMain* sc = new SceneMain();
	sc->addTo(game);
	game->run();
	delete game;
	return 0;
}
