#ifndef WORLD_HPP
#define WORLD_HPP
#include "commons.hpp"
#include "generator/ColumnGenerator.hpp"

class Column;
class Camera;
class DeferredContainer;
class Chunk;
class World : public GameObject {
	public:
		World();
		~World();

		void update(float deltaTime);
		void draw() const;
		void draw(Camera* cam) const;

		bool outOfBounds(int x, int y, int z) const;
		unsigned int getCube(int x, int y, int z) const;
		Column* getColumn(int x, int y, int z) const;
		Column* getColumn(vec3i pos) const {return getColumn(pos.x,pos.y,pos.z);}
		Chunk* getChunk(int x, int y, int z) const;
		Chunk* getChunk(vec3i pos) const {return getChunk(pos.x,pos.y,pos.z);}

		void setCube(int x, int y, int z, unsigned int cube);
	private:
		friend class Sun;
		Column* columns[WORLDSIZE][WORLDSIZE];
		ColumnGenerator generator;
		DeferredContainer* renderer;
};

#endif // WORLD_HPP
