#include "Column.hpp"
#include "Chunk.hpp"

Column::Column(int x, int z) : XPOS(x), ZPOS(z) {
	for(int i = 0; i < 8; ++i)
		chunks.push_back(new Chunk(XPOS,i,ZPOS));
	for(int i = 0; i < 8; ++i)
		for(int x = 0; x < CHUNKSIZE; ++x)
			for(int y = 0; y < CHUNKSIZE; ++y)
				for(int z = 0; z < CHUNKSIZE; ++z)
					chunks[i]->cubes[x][y][z].ID = (rand()%9);
	for(int x = 0; x < CHUNKSIZE; ++x)
		for(int z = 0; z < CHUNKSIZE; ++z)
			skyLevel[x][z] = CHUNKSIZE*8;
}

Column::~Column() {
	for(unsigned int i = 0; i < chunks.size(); ++i)
		if(chunks[i] != nullptr)
			delete chunks[i];
	chunks.resize(0);
}

unsigned int Column::getSkyLevel(unsigned int x, unsigned int z) const {
	VBE_ASSERT(x < CHUNKSIZE && z < CHUNKSIZE, "Invalid Column::getSkyLevel() parameters");
	return skyLevel[x][z];
}

Cube Column::getCube(unsigned int x, unsigned int y, unsigned int z) const {
	VBE_ASSERT(x < CHUNKSIZE && z < CHUNKSIZE, "Invalid Column::getCube() parameters");
	int chunk = y >> CHUNKSIZE_POW2;
	if((int)chunks.size() <= chunk)
		return Cube(0,MAXLIGHT);
	if(chunks[chunk] == nullptr)
		return (skyLevel[x][z] > y? Cube(0,MINLIGHT) : Cube(0,MINLIGHT), Cube(0,MAXLIGHT));
	return chunks[chunk]->cubes[x][y&CHUNKSIZE_MASK][z];
}

void Column::setCubeID(unsigned int x, unsigned int y, unsigned int z, unsigned char ID) {
	VBE_ASSERT(x < CHUNKSIZE && z < CHUNKSIZE, "Invalid Column::setCubeID() parameters");
	int chunk = y >> CHUNKSIZE_POW2;
	if((int)chunks.size() <= chunk)
		chunks.resize(chunk+1,nullptr);
	if(chunks[chunk] == nullptr)
		chunks[chunk] = new Chunk(x >> CHUNKSIZE_POW2,chunk, z >> CHUNKSIZE_POW2);
	chunks[chunk]->cubes[x][y&CHUNKSIZE_MASK][z].ID = ID;
}

void Column::setCubeLight(unsigned int x, unsigned int y, unsigned int z, unsigned char light) {
	VBE_ASSERT(x < CHUNKSIZE && z < CHUNKSIZE, "Invalid Column::setCubeID() parameters");
	int chunk = y >> CHUNKSIZE_POW2;
	if((int)chunks.size() <= chunk)
		chunks.resize(chunk+1,nullptr);
	if(chunks[chunk] == nullptr)
		chunks[chunk] = new Chunk(x >> CHUNKSIZE_POW2,chunk, z >> CHUNKSIZE_POW2);
	chunks[chunk]->cubes[x][y&CHUNKSIZE_MASK][z].light = light;
}

vec3i Column::getAbolutePos() {
	return vec3i(XPOS*CHUNKSIZE,0,ZPOS*CHUNKSIZE);
}
