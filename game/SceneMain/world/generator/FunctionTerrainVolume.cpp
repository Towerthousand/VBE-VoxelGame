#include "FunctionTerrainVolume.hpp"
#include "Function3D.hpp"

FunctionTerrainVolume::FunctionTerrainVolume(Function3D *source, unsigned int blockID) :
    source(source), blockID(blockID) {
}

FunctionTerrainVolume::~FunctionTerrainVolume() {
}

void FunctionTerrainVolume::fillData(int x, int z, unsigned int* data, GenParams* params) {
    floatType* sourceData = new floatType[GENERATIONHEIGHT*CHUNKSIZE];
    source->fillData(x, z, sourceData, params);
    for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y) {
        if(sourceData[y] <= 0.0f)
            data[y] = 0;
        else
            data[y] = blockID;
    }
    delete[] sourceData;
}
