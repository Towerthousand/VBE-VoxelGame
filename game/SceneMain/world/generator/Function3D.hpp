#ifndef FUNCTION3D_HPP
#define FUNCTION3D_HPP
#include "commons.hpp"

typedef double floatType; //put double here for more precision in world gen.
typedef std::vector<std::vector<std::vector<floatType> > > float3Data;
typedef std::vector<std::vector<floatType> > float2Data;
typedef std::vector<floatType> float1Data;

class Function3D { //abstract
    public:
        Function3D() {}
        virtual ~Function3D() {}
        //x,y,z are world coords
        virtual float3Data getFloat3Data(int x, int y, int z, int sx, int sy, int sz) = 0;
        //data returned must be sx by sy by sz
};

#endif // FUNCTION3D_HPP
