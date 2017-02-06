#include "FunctionTerrainOverlay.hpp"

FunctionTerrainOverlay::FunctionTerrainOverlay(FunctionTerrain *source, unsigned int overlayID, unsigned int surfaceID, unsigned int depth) :
    source(source), overlayID(overlayID), surfaceID(surfaceID), depth(depth) {
}

FunctionTerrainOverlay::~FunctionTerrainOverlay() {
}

void FunctionTerrainOverlay::fillData(int x, int z, unsigned int* data, GenParams* params) {
    source->fillData(x, z, data, params);
    for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE-1; ++y)
        if (data[y] == surfaceID && data[y+1] == 0)
            for (int a = 0; a < depth; ++a) {
                if (y-a >= 0 && data[y-a] == surfaceID)
                    data[y-a] = overlayID;
                else
                    break;
            }
}
