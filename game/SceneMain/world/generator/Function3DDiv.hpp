#ifndef FUNCTION3DDIV_HPP
#define FUNCTION3DDIV_HPP
#include "Function3D.hpp"

class Function3DDiv : public Function3D {
    public:
        Function3DDiv(Function3D* A, Function3D* B);
        ~Function3DDiv();
        virtual void fillData(int x, int z, floatType* data, GenParams* params) override;

    private:
        Function3D* funcA;
        Function3D* funcB;
};

#endif // FUNCTION3DDIV_HPP
