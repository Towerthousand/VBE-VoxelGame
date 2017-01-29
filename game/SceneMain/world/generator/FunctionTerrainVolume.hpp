#ifndef FUNCTIONTERRAINVOLUME_HPP
#define FUNCTIONTERRAINVOLUME_HPP
#include "FunctionTerrain.hpp"

class Function3D;
class FunctionTerrainVolume : public FunctionTerrain {
    public:
        FunctionTerrainVolume(Function3D* source, unsigned int blockID);
        virtual ~FunctionTerrainVolume();
        ID3Data getID3Data(int x, int y, int z, int sx, int sy, int sz, GenParams* params); //world coords
    private:
        Function3D* source;
        unsigned int blockID;
};

#endif // FUNCTIONTERRAINVOLUME_HPP
