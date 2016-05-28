#include "FunctionTerrrainVolume.hpp"
#include "Function3D.hpp"

FunctionTerrrainVolume::FunctionTerrrainVolume(Function3D *source, unsigned int blockID) :
    source(source), blockID(blockID) {
}

FunctionTerrrainVolume::~FunctionTerrrainVolume() {
}

ID3Data FunctionTerrrainVolume::getID3Data(int x, int y, int z, int sx, int sy, int sz) { //x, y, z are chunkgrid coords
    float3Data sourceData = source->getFloat3Data(x,y,z,sx,sy,sz);
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
