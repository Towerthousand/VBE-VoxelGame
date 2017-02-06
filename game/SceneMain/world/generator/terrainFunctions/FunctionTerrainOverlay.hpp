#ifndef FUNCTIONTERRAINOVERLAY_HPP
#define FUNCTIONTERRAINOVERLAY_HPP
#include "FunctionTerrain.hpp"

class FunctionTerrainOverlay : public FunctionTerrain {
    public:
        FunctionTerrainOverlay(FunctionTerrain* source, unsigned int overlayID, unsigned int surfaceID, unsigned int depth);
        virtual ~FunctionTerrainOverlay();
        virtual void fillData(int x, int z, unsigned int* data, GenParams* params) override;
    private:
        FunctionTerrain* source;
        unsigned int overlayID;
        unsigned int surfaceID;
        int depth;
};

#endif // FUNCTIONTERRAINOVERLAY_HPP
