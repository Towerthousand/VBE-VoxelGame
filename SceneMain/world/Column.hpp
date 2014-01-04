#ifndef COLUMN_HPP
#define COLUMN_HPP
#include "commons.hpp"
#include "Cube.hpp"

class Chunk;
class Column {
	public:
		Column(int x, int z);
		~Column();

		unsigned int getSkyLevel(unsigned int x, unsigned int z) const; //relative, 0 <= x,z < CHUNKSIZE
		Cube getCube(unsigned int x, unsigned int y, unsigned int z) const; //relative, 0 <= x,z < CHUNKSIZE
		vec3i getAbolutePos(); //in cubes
		int getX() { return XPOS; }
		int getZ() { return ZPOS; }
		std::vector<Chunk*>& getChunks() { return chunks; }

		void setCubeID(unsigned int x, unsigned int y, unsigned int z, unsigned char ID);
		void setCubeLight(unsigned int x, unsigned int y, unsigned int z, unsigned char light);
	private:
		int XPOS; //in chunks
		int ZPOS; //in chunks
		std::vector<Chunk*> chunks;
		unsigned int skyLevel[CHUNKSIZE][CHUNKSIZE];
};

#endif // COLUMN_HPP
