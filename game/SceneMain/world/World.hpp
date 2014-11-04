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

		bool outOfBounds(int x, int y, int z) const;
		bool outOfBounds(vec3i pos) const {return outOfBounds(pos.x,pos.y,pos.z);}
		unsigned int getCube(int x, int y, int z) const;
		unsigned int getCube(vec3i pos) const {return getCube(pos.x,pos.y,pos.z);}
		Column* getColumn(int x, int y, int z) const;
		Column* getColumn(vec3i pos) const {return getColumn(pos.x,pos.y,pos.z);}
		Chunk* getChunk(int x, int y, int z) const;
		Chunk* getChunk(vec3i pos) const {return getChunk(pos.x,pos.y,pos.z);}
		Column* getColumnCC(int x, int y, int z) const;
		Column* getColumnCC(vec3i pos) const {return getColumnCC(pos.x,pos.y,pos.z);}
		Chunk* getChunkCC(int x, int y, int z) const;
		Chunk* getChunkCC(vec3i pos) const {return getChunkCC(pos.x,pos.y,pos.z);}

		void setCube(int x, int y, int z, unsigned int cube);
		void setCube(vec3i pos, unsigned int cube) {setCube(pos.x, pos.y, pos.z, cube);}
	private:
		void update(float deltaTime);
		void draw() const;
		void draw(const Camera* cam) const;

		friend class Sun;
		unsigned int highestChunkY;
		Column* columns[WORLDSIZE][WORLDSIZE];
		ColumnGenerator generator;
		DeferredContainer* renderer;
};

#endif // WORLD_HPP
