#ifndef FUNCTIONTERRAINHEIGHTMAP_HPP
#define FUNCTIONTERRAINHEIGHTMAP_HPP
#include "FunctionTerrain.hpp"

class Function2D;
class FunctionTerrainHeightmap : public FunctionTerrain {
    public:
        FunctionTerrainHeightmap(Function2D* source, unsigned int blockID);
        virtual ~FunctionTerrainHeightmap();
        virtual void fillData(int x, int z, unsigned int* data, GenParams* params) override;
    private:
        Function2D* source;
        unsigned int blockID;
};

#endif // FUNCTIONTERRAINHEIGHTMAP_HPP
