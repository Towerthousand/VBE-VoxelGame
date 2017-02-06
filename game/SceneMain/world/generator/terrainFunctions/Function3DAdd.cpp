#include "Function3DAdd.hpp"

Function3DAdd::Function3DAdd(const std::vector<Function3D*>& operands): operands(operands) {
}

Function3DAdd::~Function3DAdd(){
    for(Function3D* f : operands) delete f;
}

void Function3DAdd::fillData(int x, int z, floatType* data, GenParams* params) {
    std::vector<floatType*> sources;
    for(unsigned int i = 1; i < operands.size(); ++i) {
        sources.push_back(new floatType[GENERATIONHEIGHT*CHUNKSIZE]);
        operands[i]->fillData(x, z, sources[i-1], params);
    }
    operands[0]->fillData(x, z, data, params);
    for(floatType* s : sources)
        for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
            data[y] += s[y];
    for(floatType* s : sources)
        delete[] s;
}
