#include "Function3DSimplex.hpp"

Function3DSimplex::Function3DSimplex(std::mt19937* generator, float min, float max, float scale) :
    Function3D(), noise(generator, min, max, scale) {
}

Function3DSimplex::~Function3DSimplex() {
}

float3Data Function3DSimplex::getFloat3Data(int x, int y, int z, int sx, int sy, int sz, GenParams* params) { //world coords
    (void) params;
    float3Data result(sx,float2Data(sy,float1Data(sz,0.0)));
    for(int x2 = x; x2 < x+sx; ++x2)
        for(int y2 = y; y2 < y+sy; ++y2)
            for(int z2 = z; z2 < z+sz; ++z2)
                result[x2-x][y2-y][z2-z] = noise.get(x2, y2, z2);
    return result;
}
