#ifndef FUNCTIONTERRAIN_HPP
#define FUNCTIONTERRAIN_HPP
#include "commons.hpp"
#include "GenParams.hpp"

class FunctionTerrain {//abstract
        public:
            FunctionTerrain() {}
            virtual ~FunctionTerrain() {}
            //x,z are world coords
            virtual void fillData(int x, int z, unsigned int* data, GenParams* params) = 0;
};

#endif // FUNCTIONTERRAIN_HPP
