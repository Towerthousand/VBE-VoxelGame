#ifndef COLUMN_HPP
#define COLUMN_HPP
#include "commons.hpp"
#include "Cube.hpp"

class Chunk;
class Column {
	public:
		Column(int x, int z);
		~Column();

		unsigned int getCube(unsigned int x, unsigned int y, unsigned int z) const; //relative, 0 <= x,z < CHUNKSIZE
		vec3i getAbolutePos() const; //in cubes
		int getX() const { return XPOS; }
		int getZ() const { return ZPOS; }
		const std::vector<Chunk*>& getChunks() const { return chunks; }

		void setCube(unsigned int x, unsigned int y, unsigned int z, unsigned int cube);
	private:
		int XPOS; //in chunks
		int ZPOS; //in chunks
		std::vector<Chunk*> chunks;

		friend class ColumnGenerator;
};

#endif // COLUMN_HPP
