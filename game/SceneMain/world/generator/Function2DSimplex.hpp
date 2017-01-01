#ifndef FUNCTION2DSIMPLEX_HPP
#define FUNCTION2DSIMPLEX_HPP
#include "Function2D.hpp"
#include "Noise2D.hpp"

class Function2DSimplex : public Function2D {
    public:
        Function2DSimplex(std::mt19937* generator, float min, float max, float scale);
        ~Function2DSimplex();

        float2Data getFloat2Data(int x, int z, int sx, int sz); //world coords
    private:
        Noise2D noise;
};
#endif // FUNCTION2DSIMPLEX_HPP
