#ifndef WORLD_HPP
#define WORLD_HPP
#include "commons.hpp"

class Camera;
class DeferredContainer;
class Level;
class Chunk;
class World : public GameObject {
	public:
		World();
		~World();

		void update(float deltaTime);
		void draw() const;
		void draw(Camera* cam) const;

		bool outOfBounds(int x, int y, int z) const;
		unsigned char getBlock(int x, int y, int z) const;
		void setBlock(int x, int y, int z, unsigned char block);
		Chunk* getChunk(int x, int y, int z) const;
		void setChunk(int x, int y, int z, Chunk* chunk);

		Camera* getCamera() const;

	private:

		std::vector<Chunk*> chunks;

		friend class Sun;
		Level* level;
		DeferredContainer* renderer;
};

#endif // WORLD_HPP
