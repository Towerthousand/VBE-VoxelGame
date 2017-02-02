#include "Function3DYcoord.hpp"

Function3DYcoord::Function3DYcoord() {
}

Function3DYcoord::~Function3DYcoord() {
}

float3Data Function3DYcoord::getFloat3Data(int x, int y, int z, int sx, int sy, int sz, GenParams* params) {
    (void) params;
    (void) x;
    (void) z;
    float3Data result(sx,float2Data(sy,float1Data(sz,0.0)));
    for(int localX = 0; localX < sx; ++localX)
        for(int localY = 0; localY < sy; ++localY)
            for(int localZ = 0; localZ < sz; ++localZ)
                result[localX][localY][localZ] = y+localY;
    return result;
}
