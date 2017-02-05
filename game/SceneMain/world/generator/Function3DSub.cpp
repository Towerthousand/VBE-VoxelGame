#include "Function3DSub.hpp"

Function3DSub::Function3DSub(Function3D* A, Function3D* B): funcA(A), funcB(B) {
}

Function3DSub::~Function3DSub(){
    delete funcA;
    delete funcB;
}

void Function3DSub::fillData(int x, int z, floatType* data, GenParams* params) {
    floatType* substract = new floatType[GENERATIONHEIGHT*CHUNKSIZE];
    funcA->fillData(x, z, data, params);
    funcB->fillData(x, z, substract, params);
    for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
        data[y] -= substract[y];
    delete[] substract;
}
