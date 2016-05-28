#include "Function2DConst.hpp"

Function2DConst::Function2DConst(float val) : val(val) {
}

Function2DConst::~Function2DConst() {
}

float2Data Function2DConst::getFloat2Data(int x, int y, int sx, int sy) {
    (void) x;
    (void) y;
    return float2Data(sx,float1Data(sy,val));
}
