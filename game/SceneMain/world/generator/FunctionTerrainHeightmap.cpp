#include "FunctionTerrainHeightmap.hpp"
#include "Function2D.hpp"

FunctionTerrainHeightmap::FunctionTerrainHeightmap(Function2D *source, unsigned int blockID) :
    source(source), blockID(blockID) {
}

FunctionTerrainHeightmap::~FunctionTerrainHeightmap() {
}

void FunctionTerrainHeightmap::fillData(int x, int z, unsigned int* data, GenParams* params) {
    for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
        data[y] = source->getValue(x, z, params) < y? 0 : blockID;
}

