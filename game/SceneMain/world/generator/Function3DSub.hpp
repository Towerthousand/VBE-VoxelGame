#ifndef FUNCTION3DSUBSTRACT_HPP
#define FUNCTION3DSUBSTRACT_HPP
#include "Function3D.hpp"

class Function3DSub : public Function3D {
    public:
        Function3DSub(Function3D* A, Function3D* B);
        ~Function3DSub();
        float3Data getFloat3Data(int x, int y, int z, int sx, int sy, int sz, GenParams* params); //world coords

    private:
        Function3D* funcA;
        Function3D* funcB;
};

#endif // FUNCTION3DSUBSTRACT_HPP
