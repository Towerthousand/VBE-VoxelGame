#include "Function3DSub.hpp"

Function3DSub::Function3DSub(Function3D* A, Function3D* B): funcA(A), funcB(B) {
}

Function3DSub::~Function3DSub(){
	delete funcA;
	delete funcB;
}

float3Data Function3DSub::getFloat3Data(int x, int y, int z, int sx, int sy, int sz) {
	float3Data dataA = funcA->getFloat3Data(x,y,z,sx,sy,sz);
	float3Data dataB = funcB->getFloat3Data(x,y,z,sx,sy,sz);
	float3Data result(sx,float2Data(sy,float1Data(sz,0.0)));
	for(int i = 0; i < sx; ++i)
		for(int j = 0; j < sy; ++j)
			for(int k = 0; k < sz; ++k)
				result[i][j][k] = dataA[i][j][k] - dataB[i][j][k];
	return result;
}
