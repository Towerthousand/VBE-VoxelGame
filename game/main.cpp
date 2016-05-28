#include "SceneMain/SceneMain.hpp"

int main() {
    //Log::setFlags(Log::Timestamp);
    ContextSettings s;
    s.versionMajor = 4;
    s.versionMinor = 3;
    Game* game = new Game(Window::getFullscreenModes()[0], s);
    game->setFixedUpdateRate(20);
    SceneMain* sc = new SceneMain();
    sc->addTo(game);
    game->run();
    delete game;
    return 0;
}
