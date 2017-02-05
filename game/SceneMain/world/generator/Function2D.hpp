#ifndef FUNCTION2D_HPP
#define FUNCTION2D_HPP
#include "Function3D.hpp"

class Function2D : public Function3D{ //abstract
    public:
        Function2D() : Function3D() {}
        virtual ~Function2D() {}
        //x,z are world coords
        virtual floatType getValue(int x, int z, GenParams* params) = 0;
        virtual void fillData(int x, int z, floatType* data, GenParams* params) override final {
            floatType val = getValue(x, z, params);
            for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
                data[y] = val;
        }
};

#endif // FUNCTION2D_HPP
