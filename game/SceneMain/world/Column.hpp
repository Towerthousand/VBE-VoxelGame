#ifndef COLUMN_HPP
#define COLUMN_HPP
#include "commons.hpp"

class Chunk;
class Column {
	public:
		Column(int x, int z);
		~Column();

		unsigned int getCube(unsigned int x, unsigned int y, unsigned int z) const;
		inline vec3i getAbolutePos() const { //in cubes
			return vec3i(XPOS*CHUNKSIZE,0,ZPOS*CHUNKSIZE);
		}
		inline int getX() const { return XPOS; }
		inline int getZ() const { return ZPOS; }
		inline unsigned int getChunkCount() const { return chunks.size(); }
		Chunk* getChunk(int y) const {
			int realY = y >> CHUNKSIZE_POW2;
			return (realY < 0 || realY >= (int)chunks.size()) ? nullptr : chunks[realY];
		}
		inline Chunk* getChunkCC(int y) const {
			return (y < 0 || y >= (int)chunks.size()) ? nullptr : chunks[y];
		}

		void rebuildAllMeshes();
		void setCube(unsigned int x, unsigned int y, unsigned int z, unsigned int cube);
		void rebuildMesh(int y);
		void rebuildMeshCC(int y);
	private:
		int XPOS; //in chunks
		int ZPOS; //in chunks
		std::vector<Chunk*> chunks;

		friend class ColumnGenerator;
};

#endif // COLUMN_HPP
