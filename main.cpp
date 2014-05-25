#include "SceneMain/SceneMain.hpp"
#include "SceneMain/NetworkManager.hpp"

using namespace std;

int main() {

	Environment::setup().windowTitle = ":3";
	Environment::setup().windowWidth = 1536;
	Environment::setup().windowHeight = Environment::setup().windowWidth*9/16;
	Environment::setup().windowFlags = Screen::WINDOW_SHOWN | Screen::WINDOW_OPENGL; // | Screen::WINDOW_FULLSCREEN;
	Environment::setup().mouseGrab = true;

	Game* game = new Game();

	NetworkManager* n = new NetworkManager("dirbaio.net", 25565, "blarg");
	n->addTo(game);

	game->run();
	delete game;
	return 0;
}
