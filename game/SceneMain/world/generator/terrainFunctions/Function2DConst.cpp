#include "Function2DConst.hpp"

Function2DConst::Function2DConst(float val) : val(val) {
}

Function2DConst::~Function2DConst() {
}

floatType Function2DConst::getValue(int x, int z, GenParams* params) {
    (void) x;
    (void) z;
    (void) params;
    return val;
}
