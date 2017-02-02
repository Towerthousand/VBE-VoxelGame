#include "FunctionTerrainHeightmap.hpp"
#include "Function2D.hpp"

FunctionTerrainHeightmap::FunctionTerrainHeightmap(Function2D *source, unsigned int blockID) :
    source(source), blockID(blockID) {
}

FunctionTerrainHeightmap::~FunctionTerrainHeightmap() {
}

ID3Data FunctionTerrainHeightmap::getID3Data(int x, int y, int z, int sx, int sy, int sz, GenParams* params) { //x, y, z are chunkgrid coords
    (void) params;
    float2Data sourceData = source->getFloat2Data(x,z,sx,sz,params);
    ID3Data result(sx,std::vector<std::vector<unsigned int> >(sy,std::vector<unsigned int>(sz,0)));
    for(int localX = 0; localX < sx; ++localX)
        for(int localY = 0; localY < sy; ++localY)
            for(int localZ = 0; localZ < sz; ++localZ) {
                if (sourceData[localX][localZ] < (y+localY))
                    result[localX][localY][localZ] = 0;
                else
                    result[localX][localY][localZ] = blockID;
            }
    return result;
}

