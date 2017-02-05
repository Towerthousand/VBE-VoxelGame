#ifndef FUNCTION3D_HPP
#define FUNCTION3D_HPP
#include "commons.hpp"
#include "GenParams.hpp"

typedef double floatType; //put double here for more precision in world gen.

class Function3D { //abstract
    public:
        Function3D() {}
        virtual ~Function3D() {}
        //x,z are world coords
        virtual void fillData(int x, int z, floatType* data, GenParams* params) = 0;
};

#endif // FUNCTION3D_HPP
