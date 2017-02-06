#ifndef FUNCTIONTERRAINVOLUME_HPP
#define FUNCTIONTERRAINVOLUME_HPP
#include "FunctionTerrain.hpp"

class Function3D;
class FunctionTerrainVolume : public FunctionTerrain {
    public:
        FunctionTerrainVolume(Function3D* source, unsigned int blockID);
        virtual ~FunctionTerrainVolume();
        virtual void fillData(int x, int z, unsigned int* data, GenParams* params) override;
    private:
        Function3D* source;
        unsigned int blockID;
};

#endif // FUNCTIONTERRAINVOLUME_HPP
