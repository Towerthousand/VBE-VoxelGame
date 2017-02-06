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
    if(glm::abs(ma-width+2) < 1 && glm::abs(mb-width-3) < 3) return 1000;
    return 0;
}

void Function3DHelix::fillData(int x, int z, floatType* data, GenParams* params) {
    (void) params;
    for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
        data[y] = helix(x, y, z);
}

