#ifndef FUNCTION3DSUBSTRACT_HPP
#define FUNCTION3DSUBSTRACT_HPP
#include "Function3D.hpp"

class Function3DSub : public Function3D {
    public:
        Function3DSub(Function3D* A, Function3D* B);
        ~Function3DSub();
        virtual void fillData(int x, int z, floatType* data, GenParams* params) override;

    private:
        Function3D* funcA;
        Function3D* funcB;
};

#endif // FUNCTION3DSUBSTRACT_HPP
