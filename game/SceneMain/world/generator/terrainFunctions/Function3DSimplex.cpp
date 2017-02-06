#include "Function3DSimplex.hpp"

Function3DSimplex::Function3DSimplex(std::mt19937* generator, float min, float max, float scale) :
    Function3D(), noise(generator, min, max, scale) {
}

Function3DSimplex::~Function3DSimplex() {
}

void Function3DSimplex::fillData(int x, int z, floatType* data, GenParams* params) {
    (void) params;
    for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
        data[y] += noise.get(x, y, z);
}
