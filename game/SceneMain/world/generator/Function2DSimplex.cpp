#include "Function2DSimplex.hpp"

Function2DSimplex::Function2DSimplex(std::mt19937 *generator, float min, float max, float scale) :
    Function2D(), noise(generator, min, max, scale) {
}

Function2DSimplex::~Function2DSimplex() {
}

float2Data Function2DSimplex::getFloat2Data(int x, int z, int sx, int sz, GenParams* params) { //world coords
    (void) params;
    float2Data result(sx,float1Data(sz,0.0));
    for(int x2 = x; x2 < x+sx; ++x2)
        for(int z2 = z; z2 < z+sz; ++z2)
            result[x2-x][z2-z] = noise.get(x2, z2);
    return result;
}
