#ifndef FUNCTION3DYCOORD_HPP
#define FUNCTION3DYCOORD_HPP
#include "Function3D.hpp"

class Function3DYcoord : public Function3D {
    public:
        Function3DYcoord();
        ~Function3DYcoord();
        virtual void fillData(int x, int z, floatType* data, GenParams* params) override;
};

#endif // FUNCTION3DYCOORD_HPP
