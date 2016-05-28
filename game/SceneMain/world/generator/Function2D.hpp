#ifndef FUNCTION2D_HPP
#define FUNCTION2D_HPP
#include "Function3D.hpp"

class Function2D : public Function3D{ //abstract
    public:
        Function2D() : Function3D() {}
        virtual ~Function2D() {}
        //x,y,z are world coords
        virtual float2Data getFloat2Data(int x, int z, int sx, int sz) = 0;
        //data returned must be sx by sy
        float3Data getFloat3Data(int x, int y, int z, int sx, int sy, int sz) {
            (void) y;
            float2Data layer = getFloat2Data(x,z,sx,sz);
            float3Data result(sx,float2Data(sy,float1Data(sz,0.0)));
            for(int i = 0; i < sx; ++i)
                for(int j = 0; j < sy; ++j)
                    for(int k = 0; k < sz; ++k)
                        result[i][j][k] = layer[i][k];
            return result;
        }
        //data returned must be sx by sy by sz
};

#endif // FUNCTION2D_HPP
