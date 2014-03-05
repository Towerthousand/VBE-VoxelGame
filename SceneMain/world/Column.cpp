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
	int chunk = y >> CHUNKSIZE_POW2;
	if((int)chunks.size() <= chunk || chunks[chunk] == nullptr)
		return 0;
	return chunks[chunk]->cubes[x][y&CHUNKSIZE_MASK][z];
}

void Column::setCube(unsigned int x, unsigned int y, unsigned int z, unsigned int cube) {
	VBE_ASSERT(int(x) < CHUNKSIZE && int(z) < CHUNKSIZE, "Invalid Column::setCubeID() parameters");
	int chunk = y >> CHUNKSIZE_POW2;
	if((int)chunks.size() <= chunk)
		chunks.resize(chunk+1,nullptr);
	if(chunks[chunk] == nullptr)
		chunks[chunk] = new Chunk(XPOS, chunk, ZPOS);
	chunks[chunk]->cubes[x][y&CHUNKSIZE_MASK][z] = cube;
	chunks[chunk]->markedForRedraw = true;
}

void Column::rebuildMeshes() {
	return;
	for(unsigned int i = 0; i < chunks.size(); ++i)
		if(chunks[i] != nullptr)
			chunks[i]->markedForRedraw = true;
}

vec3i Column::getAbolutePos() const {
	return vec3i(XPOS*CHUNKSIZE,0,ZPOS*CHUNKSIZE);
}
