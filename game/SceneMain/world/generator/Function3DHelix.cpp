#include "Function3DHelix.hpp"

Function3DHelix::Function3DHelix(float period, float width, float range, float offset, float yoffset, float zoffset, float tiling, float sin, float sin2) :
    period(period),
    width(width),
    range(range),
    offset(offset),
    yoffset(yoffset),
    zoffset(zoffset),
    tiling(tiling),
    sin(sin),
    sin2(sin2)
    {
}

Function3DHelix::~Function3DHelix() {
}

float Function3DHelix::helix(float x, float y, float z) {
    float mc = x; //direction of the helix
    float ma = glm::mod(y-yoffset+sin*glm::sin(mc/sin2),width*tiling); //perp vector 2
    float mb = glm::mod(z-zoffset,width*tiling); //perp vector 2
    float cx = ma - width - glm::cos((mc+offset)/period) * width;
    float cz = mb - width - glm::sin((mc+offset)/period) * width;
    if(cx > 0 && cz > 0 && cx < range && cz < range) return 1000;
    if(abs(ma-width+2) < 1 && abs(mb-width-3) < 3) return 1000;
    return 0;
}

float3Data Function3DHelix::getFloat3Data(int x, int y, int z, int sx, int sy, int sz, GenParams* params) {
    (void) params;
    float3Data result(sx,float2Data(sy,float1Data(sz,0.0)));
    for(int localX = 0; localX < sx; ++localX)
        for(int localY = 0; localY < sy; ++localY)
            for(int localZ = 0; localZ < sz; ++localZ)
                result[localX][localY][localZ] = helix(x+localX, y+localY, z+localZ);
    return result;
}

