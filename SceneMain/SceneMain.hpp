#ifndef SCENEMAIN_HPP
#define SCENEMAIN_HPP
#include "commons.hpp"
#include<map>

class World;
class Player;
class PlayerBase;
class SceneMain : public GameObject {
	public:
		SceneMain();
		~SceneMain();
		void update(float deltaTime);

		World* world;
		Player* player;
		std::map<int, PlayerBase*> players;

		void spawnPlayer(int id);
		void despawnPlayer(int id);
		PlayerBase* getPlayer(int id);

	private:
        void loadResources();
		float debugCounter;
		int fpsCount;
};

#endif // SCENEMAIN_HPP
