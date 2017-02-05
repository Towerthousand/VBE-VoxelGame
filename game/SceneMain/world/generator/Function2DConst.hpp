#ifndef FUNCTION3DCONSTANT_HPP
#define FUNCTION3DCONSTANT_HPP
#include "Function2D.hpp"

class Function2DConst : public Function2D {
    public:
        Function2DConst(float val);
        ~Function2DConst();
        floatType getValue(int x, int z, GenParams* params) override;

    private:
        float val;
};

#endif // FUNCTION3DCONSTANT_HPP
