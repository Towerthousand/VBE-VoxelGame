#include "Function3DAdd.hpp"

Function3DAdd::Function3DAdd(const std::vector<Function3D*>& operands): operands(operands) {
}

Function3DAdd::~Function3DAdd(){
	for(Function3D* f : operands) delete f;
}

float3Data Function3DAdd::getFloat3Data(int x, int y, int z, int sx, int sy, int sz) {
	std::vector<float3Data> data;
	for(unsigned int i = 0; i < operands.size(); ++i) data.push_back(operands[i]->getFloat3Data(x, y, z, sx, sy, sz));
	float3Data result(sx,float2Data(sy,float1Data(sz,0.0)));
	for(float3Data d : data)
		for(int i = 0; i < sx; ++i)
			for(int j = 0; j < sy; ++j)
				for(int k = 0; k < sz; ++k)
					result[i][j][k] += d[i][j][k];
	return result;
}
