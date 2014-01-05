#include "FunctionTerrainOverlay.hpp"

FunctionTerrainOverlay::FunctionTerrainOverlay(FunctionTerrain *source, char overlayID, char surfaceID, int depth) :
	source(source), overlayID(overlayID), surfaceID(surfaceID), depth(depth) {
}

FunctionTerrainOverlay::~FunctionTerrainOverlay() {
}

ID3Data FunctionTerrainOverlay::getID3Data(int x, int y, int z, int sx, int sy, int sz) { //x, y, z are chunkgrid coords
	ID3Data src = source->getID3Data(x,y,z,sx,sy,sz);
	for(int i = 0; i < sx; ++i)
		for(int j = 0; j < sy-1; ++j)
			for(int k = 0; k < sz; ++k)
				if (src[i][j][k] == surfaceID && src[i][j+1][k] == 0)
					for (int a = 0; a < depth; ++a) {
						if ( (j-a)>= 0 && src[i][j-a][k] == surfaceID)
							src[i][j-a][k] = overlayID;
						else
							break;
					}
	return src;
}
