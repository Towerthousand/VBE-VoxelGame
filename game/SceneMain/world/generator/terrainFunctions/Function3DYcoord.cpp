#include "Function3DYcoord.hpp"

Function3DYcoord::Function3DYcoord() {
}

Function3DYcoord::~Function3DYcoord() {
}

void Function3DYcoord::fillData(int x, int z, floatType* data, GenParams* params) {
    (void) params;
    (void) x;
    (void) z;
    for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
        data[y] = y;
}
