#ifndef WORLD_HPP
#define WORLD_HPP
#include "commons.hpp"

class World : public GameObject {
	public:
		World();
		~World();

	private:
		//Chunk* chunks[WORLDSIZE][WORLDSIZE];
};

#endif // WORLD_HPP
