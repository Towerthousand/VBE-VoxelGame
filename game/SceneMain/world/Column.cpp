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
	VBE_ASSERT(int(x) < CHUNKSIZE && int(z) < CHUNKSIZE, "Invalid Column::setCube() parameters");
	int chunk = y >> CHUNKSIZE_POW2;
	if((int)chunks.size() <= chunk)
		chunks.resize(chunk+1, nullptr);
	if(chunks[chunk] == nullptr)
		chunks[chunk] = new Chunk(XPOS, chunk, ZPOS);
	chunks[chunk]->cubes[x][y&CHUNKSIZE_MASK][z] = cube;
}

void Column::rebuildAllMeshes() {
	for(unsigned int i = 0; i < chunks.size(); ++i)
		if(chunks[i] != nullptr)
			chunks[i]->needsMeshRebuild = true;
}

void Column::rebuildMesh(int y) {
	Chunk* c = getChunk(y);
	if(c) c->needsMeshRebuild = true;
}

void Column::rebuildMeshCC(int y) {
	Chunk* c = getChunkCC(y);
	if(c) c->needsMeshRebuild = true;
}
