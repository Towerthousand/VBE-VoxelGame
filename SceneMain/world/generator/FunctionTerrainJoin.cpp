#include "FunctionTerrainJoin.hpp"

FunctionTerrainJoin::FunctionTerrainJoin(FunctionTerrain *A, FunctionTerrain *B) :
	funcA(A), funcB(B){
}

FunctionTerrainJoin::~FunctionTerrainJoin() {
	delete funcA;
	delete funcB;
}

ID3Data FunctionTerrainJoin::getID3Data(int x, int y, int z, int sx, int sy, int sz) { //x, y, z are chunkgrid coords
	ID3Data dataA = funcA->getID3Data(x,y,z,sx,sy,sz);
	ID3Data dataB = funcB->getID3Data(x,y,z,sx,sy,sz);
	ID3Data result(sx,std::vector<std::vector<char> >(sy,std::vector<char>(sz,0)));
	for(int i = 0; i < sx; ++i)
		for(int j = 0; j < sy; ++j)
			for(int k = 0; k < sz; ++k) {
				if(dataA[i][j][k] != 0)
					result[i][j][k] = dataA[i][j][k];
				else if(dataB[i][j][k] != 0)
					result[i][j][k] = dataB[i][j][k];
				else
					result[i][j][k] = 0;
			}
	return result;
}
