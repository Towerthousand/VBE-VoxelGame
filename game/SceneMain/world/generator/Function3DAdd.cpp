#include "Function3DAdd.hpp"

Function3DAdd::Function3DAdd(const std::vector<Function3D*>& operands): operands(operands) {
}

Function3DAdd::~Function3DAdd(){
    for(Function3D* f : operands) delete f;
}

float3Data Function3DAdd::getFloat3Data(int x, int y, int z, int sx, int sy, int sz) {
    std::vector<float3Data> data;
    for(unsigned int i = 1; i < operands.size(); ++i) data.push_back(operands[i]->getFloat3Data(x, y, z, sx, sy, sz));
    float3Data ret = operands[0]->getFloat3Data(x, y, z, sx, sy, sz);
    for(unsigned int d = 1; d < data.size(); ++d)
        for(int i = 0; i < sx; ++i)
            for(int j = 0; j < sy; ++j)
                for(int k = 0; k < sz; ++k)
                    ret[i][j][k] += data[d][i][j][k];
    return ret;
}
