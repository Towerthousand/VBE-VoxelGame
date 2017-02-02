#ifndef FUNCTION3DSIMPLEX_HPP
#define FUNCTION3DSIMPLEX_HPP
#include "Function3D.hpp"
#include "Noise3D.hpp"

class Function3DSimplex : public Function3D {
    public:
        Function3DSimplex(std::mt19937* generator, float min, float max, float scale);
        ~Function3DSimplex();

        float3Data getFloat3Data(int x, int y, int z, int sx, int sy, int sz, GenParams* params); //world coords
    private:
        Noise3D noise;
};

#endif // FUNCTION3DSIMPLEX_HPP
