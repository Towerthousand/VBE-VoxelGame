#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include "Socket.hpp"
#include<vector>
#include<string>
#include "commons.hpp"
#include "world/Level.hpp"

class SceneMain;
class World;
class NetworkManager : public GameObject
{
	private:
		Socket s;

		std::vector<unsigned char> levelData;

		SceneMain* scene;
		World* world;

	public:
		Level level;
		NetworkManager(std::string host, int port, std::string nickname);
		virtual void update(float deltaTime);

		void sendSetBlock(int x, int y, int z, bool create, unsigned char block);
		void sendPosition(float x, float y, float z, float yaw, float pitch);
};

#endif // NETWORKMANAGER_HPP
