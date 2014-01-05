#ifndef FUNCTIONTERRAINOVERLAY_HPP
#define FUNCTIONTERRAINOVERLAY_HPP
#include "FunctionTerrain.hpp"

class FunctionTerrainOverlay : public FunctionTerrain {
	public:
		FunctionTerrainOverlay(FunctionTerrain* source, char overlayID, char surfaceID, int depth);
		virtual ~FunctionTerrainOverlay();
		ID3Data getID3Data(int x, int y, int z, int sx, int sy, int sz); //world coords
	private:
		FunctionTerrain* source;
		char overlayID;
		char surfaceID;
		int depth;
};

#endif // FUNCTIONTERRAINOVERLAY_HPP
