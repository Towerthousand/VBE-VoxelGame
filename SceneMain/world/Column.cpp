#include "Column.hpp"
#include "Chunk.hpp"

Column::Column(int x, int z) : XPOS(x), ZPOS(z) {
}

Column::~Column() {
	for(unsigned int i = 0; i < chunks.size(); ++i)
		if(chunks[i] != nullptr)
			delete chunks[i];
	chunks.resize(0);
}

unsigned int Column::getCube(unsigned int x, unsigned int y, unsigned int z) const {
	VBE_ASSERT(int(x) < CHUNKSIZE && int(z) < CHUNKSIZE, "Invalid Column::getCube() parameters");
	Chunk* c = getChunk(y);
	return (c != nullptr? c->cubes[x][y&CHUNKSIZE_MASK][z] : 0);
}

void Column::setCube(unsigned int x, unsigned int y, unsigned int z, unsigned int cube) {
	VBE_ASSERT(int(x) < CHUNKSIZE && int(z) < CHUNKSIZE, "Invalid Column::setCubeID() parameters");
	int chunk = y >> CHUNKSIZE_POW2;
	if((int)chunks.size() <= chunk)
		chunks.resize(chunk+1,nullptr);
	if(chunks[chunk] == nullptr)
		chunks[chunk] = new Chunk(XPOS, chunk, ZPOS);
	chunks[chunk]->cubes[x][y&CHUNKSIZE_MASK][z] = cube;
	chunks[chunk]->needsMeshRebuild = true;
}

void Column::rebuildMeshes() {
	for(unsigned int i = 0; i < chunks.size(); ++i)
		if(chunks[i] != nullptr)
			chunks[i]->needsMeshRebuild = true;
}

vec3i Column::getAbolutePos() const {
	return vec3i(XPOS*CHUNKSIZE,0,ZPOS*CHUNKSIZE);
}

Chunk* Column::getChunk(int y) const {
	int realY = y >> CHUNKSIZE_POW2;
	return (realY < 0 || realY >= (int)chunks.size()) ? nullptr : chunks[realY];
}

Chunk*Column::getChunkCC(int y) const {
	return (y < 0 || y >= (int)chunks.size()) ? nullptr : chunks[y];
}
