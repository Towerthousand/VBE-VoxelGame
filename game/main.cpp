#include "SceneMain/SceneMain.hpp"

int main() {
	Window w(Window::getFullscreenModes()[0]);
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
