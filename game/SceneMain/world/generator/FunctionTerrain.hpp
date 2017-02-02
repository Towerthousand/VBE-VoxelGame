#ifndef FUNCTIONTERRAIN_HPP
#define FUNCTIONTERRAIN_HPP
#include "commons.hpp"
#include "GenParams.hpp"

typedef std::vector<std::vector<std::vector<unsigned int> > > ID3Data;

class FunctionTerrain {//abstract
        public:
            FunctionTerrain() {}
            virtual ~FunctionTerrain() {}
            //x,y,z are world coords
            virtual ID3Data getID3Data(int x, int y, int z, int sx, int sy, int sz, GenParams* params) = 0;
            //data returned must be sx by sy by sz
};

#endif // FUNCTIONTERRAIN_HPP
