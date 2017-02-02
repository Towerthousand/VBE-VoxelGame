#ifndef FUNCTIONTERRAINOVERLAY_HPP
#define FUNCTIONTERRAINOVERLAY_HPP
#include "FunctionTerrain.hpp"

class FunctionTerrainOverlay : public FunctionTerrain {
    public:
        FunctionTerrainOverlay(FunctionTerrain* source, unsigned int overlayID, unsigned int surfaceID, unsigned int depth);
        virtual ~FunctionTerrainOverlay();
        ID3Data getID3Data(int x, int y, int z, int sx, int sy, int sz, GenParams* params); //world coords
    private:
        FunctionTerrain* source;
        unsigned int overlayID;
        unsigned int surfaceID;
        int depth;
};

#endif // FUNCTIONTERRAINOVERLAY_HPP
