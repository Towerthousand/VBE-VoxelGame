#ifndef FUNCTIONTERRRAINVOLUME_HPP
#define FUNCTIONTERRRAINVOLUME_HPP
#include "FunctionTerrain.hpp"

class Function3D;
class FunctionTerrrainVolume : public FunctionTerrain {
	public:
		FunctionTerrrainVolume(Function3D* source, char blockID);
		virtual ~FunctionTerrrainVolume();
		ID3Data getID3Data(int x, int y, int z, int sx, int sy, int sz); //world coords
	private:
		Function3D* source;
		char blockID;
};

#endif // FUNCTIONTERRRAINVOLUME_HPP
