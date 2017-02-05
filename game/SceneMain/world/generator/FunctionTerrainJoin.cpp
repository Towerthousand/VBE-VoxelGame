#include "FunctionTerrainJoin.hpp"

FunctionTerrainJoin::FunctionTerrainJoin(FunctionTerrain *A, FunctionTerrain *B) :
    funcA(A), funcB(B){
}

FunctionTerrainJoin::~FunctionTerrainJoin() {
    delete funcA;
    delete funcB;
}

void FunctionTerrainJoin::fillData(int x, int z, unsigned int* data, GenParams* params) {
    funcA->fillData(x, z, data, params);
    unsigned int* dataB = new unsigned int[GENERATIONHEIGHT*CHUNKSIZE];
    funcB->fillData(x, z, dataB, params);
    for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
        data[y] = data[y] != 0? data[y] : (dataB[y] != 0? dataB[y] : 0 );
    delete[] dataB;
}
