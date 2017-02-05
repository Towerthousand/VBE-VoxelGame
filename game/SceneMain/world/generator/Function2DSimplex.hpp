#ifndef FUNCTION2DSIMPLEX_HPP
#define FUNCTION2DSIMPLEX_HPP
#include "Function2D.hpp"
#include "Noise2D.hpp"

class Function2DSimplex : public Function2D {
    public:
        Function2DSimplex(std::mt19937* generator, float GenParams::* min, float GenParams::* max, float GenParams::* scale);
        ~Function2DSimplex();

        floatType getValue(int x, int z, GenParams* params) override;
    private:
        Noise2D noise;

        float GenParams::* min;
        float GenParams::* max;
        float GenParams::* scale;
};
#endif // FUNCTION2DSIMPLEX_HPP
