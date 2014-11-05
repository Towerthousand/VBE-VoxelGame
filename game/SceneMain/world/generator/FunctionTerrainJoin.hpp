#ifndef FUNCTIONTERRAINJOIN_HPP
#define FUNCTIONTERRAINJOIN_HPP
#include "FunctionTerrain.hpp"

class FunctionTerrainJoin : public FunctionTerrain {
	public:
		FunctionTerrainJoin(FunctionTerrain* A, FunctionTerrain* B);
		virtual ~FunctionTerrainJoin();
		ID3Data getID3Data(int x, int y, int z, int sx, int sy, int sz); //world coords
	private:
		FunctionTerrain* funcA;
		FunctionTerrain* funcB;
};

#endif // FUNCTIONTERRAINJOIN_HPP
