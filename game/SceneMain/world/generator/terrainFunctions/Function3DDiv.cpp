#include "Function3DDiv.hpp"

Function3DDiv::Function3DDiv(Function3D* A, Function3D* B): funcA(A), funcB(B) {
}

Function3DDiv::~Function3DDiv(){
    delete funcA;
    delete funcB;
}

void Function3DDiv::fillData(int x, int z, floatType* data, GenParams* params) {
    floatType* divisor = new floatType[GENERATIONHEIGHT*CHUNKSIZE];
    funcA->fillData(x, z, data, params);
    funcB->fillData(x, z, divisor, params);
    for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
        data[y] /= divisor[y];
    delete[] divisor;
}
