#include "Function2DSimplex.hpp"

Function2DSimplex::Function2DSimplex(std::mt19937 *generator, float GenParams::* min, float GenParams::* max, float GenParams::* scale) :
    Function2D(), noise(generator), min(min), max(max), scale(scale) {
}

Function2DSimplex::~Function2DSimplex() {
}

floatType Function2DSimplex::getValue(int x, int z, GenParams* params) {
    return params->*min + (params->*max-params->*min)*noise.get(x/params->*scale, z/params->*scale);
}
