#include "FunctionTerrainVolume.hpp"
#include "Function3D.hpp"

FunctionTerrainVolume::FunctionTerrainVolume(Function3D *source, unsigned int blockID) :
    source(source), blockID(blockID) {
}

FunctionTerrainVolume::~FunctionTerrainVolume() {
}

ID3Data FunctionTerrainVolume::getID3Data(int x, int y, int z, int sx, int sy, int sz, GenParams* params) { //x, y, z are chunkgrid coords
    (void) params;
    float3Data sourceData = source->getFloat3Data(x,y,z,sx,sy,sz,params);
    ID3Data result(sx,std::vector<std::vector<unsigned int> >(sy,std::vector<unsigned int>(sz,0)));
    for(int i = 0; i < sx; ++i)
        for(int j = 0; j < sy; ++j)
            for(int k = 0; k < sz; ++k) {
                if (sourceData[i][j][k] <= 0)
                    result[i][j][k] = 0;
                else
                    result[i][j][k] = blockID;
            }
    return result;
}
