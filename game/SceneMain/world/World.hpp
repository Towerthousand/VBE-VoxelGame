#ifndef WORLD_HPP
#define WORLD_HPP
#include "commons.hpp"
#include "generator/ColumnGenerator.hpp"
#include "Column.hpp"

class Camera;
class DeferredContainer;
class World : public GameObject {
	public:
		World();
		~World();

		inline bool outOfBounds(int x, int y, int z) const {
			Column* c = columns[(x >> CHUNKSIZE_POW2) & WORLDSIZE_MASK][(z >> CHUNKSIZE_POW2) & WORLDSIZE_MASK];
			return (c == nullptr || c->getX() != (x >> CHUNKSIZE_POW2) || c->getZ() != (z >> CHUNKSIZE_POW2) || y < 0);
		}
		inline bool outOfBounds(vec3i pos) const {return outOfBounds(pos.x,pos.y,pos.z);}
		inline unsigned int getCube(int x, int y, int z) const {
			Column* c = getColumn(x,y,z);
			return (c == nullptr)? 0 : c->getCube(x & CHUNKSIZE_MASK,y,z & CHUNKSIZE_MASK);
		}
		inline unsigned int getCube(vec3i pos) const {return getCube(pos.x,pos.y,pos.z);}
		inline Column* getColumn(int x, int y, int z) const {
			Column* c = columns[(x >> CHUNKSIZE_POW2) & WORLDSIZE_MASK][(z >> CHUNKSIZE_POW2) & WORLDSIZE_MASK];
			return (c == nullptr || c->getX() != (x >> CHUNKSIZE_POW2) || c->getZ() != (z >> CHUNKSIZE_POW2) || y < 0)? nullptr:c;
		}
		inline Column* getColumn(vec3i pos) const {return getColumn(pos.x,pos.y,pos.z);}
		inline Chunk* getChunk(int x, int y, int z) const {
			Column* c = getColumn(x,y,z);
			return c == nullptr ? nullptr : c->getChunk(y);
		}
		inline Chunk* getChunk(vec3i pos) const {return getChunk(pos.x,pos.y,pos.z);}
		inline Column* getColumnCC(int x, int y, int z) const {
			Column* c = columns[x & WORLDSIZE_MASK][z & WORLDSIZE_MASK];
			return (c == nullptr || c->getX() != x || c->getZ() != z || y < 0)? nullptr:c;
		}
		inline Column* getColumnCC(vec3i pos) const {return getColumnCC(pos.x,pos.y,pos.z);}
		inline Chunk* getChunkCC(int x, int y, int z) const {
			Column* c = getColumnCC(x,y,z);
			return c == nullptr ? nullptr : c->getChunkCC(y);
		}
		inline Chunk* getChunkCC(vec3i pos) const {return getChunkCC(pos.x,pos.y,pos.z);}
		void setCube(int x, int y, int z, unsigned int cube);
		void setCubeRange(int x, int y, int z, unsigned int sx, unsigned int sy, unsigned int sz, unsigned int cube);
		inline void setCube(vec3i pos, unsigned int cube) {setCube(pos.x, pos.y, pos.z, cube);}
		inline void setCubeRange(vec3i pos, vec3i size, unsigned int cube) {
			setCubeRange(pos.x, pos.y, pos.z, size.x, size.y, size.z, cube);
		}

		inline void recalc(int x, int y, int z) {
			Column* c = getColumn(x,y,z);
			if(c) c->rebuildMesh(y);
		}
		inline void recalc(vec3i pos) {recalc(pos.x, pos.y, pos.z);}
	private:
		void update(float deltaTime);
		void draw() const;
		void draw(const Camera* cam) const;

		inline void setCubeData(int x, int y, int z, unsigned int cube) {
			Column* c = getColumn(x,y,z);
			if(c) c->setCube(x & CHUNKSIZE_MASK, y, z & CHUNKSIZE_MASK, cube);
		}

		friend class Sun;
		unsigned int highestChunkY;
		Column* columns[WORLDSIZE][WORLDSIZE];
		ColumnGenerator generator;
		DeferredContainer* renderer;
};

#endif // WORLD_HPP
