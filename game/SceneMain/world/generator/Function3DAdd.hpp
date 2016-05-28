#ifndef FUNCTION3DADD_HPP
#define FUNCTION3DADD_HPP
#include "Function3D.hpp"

class Function3DAdd : public Function3D {
    public:
        Function3DAdd(const std::vector<Function3D*>& operands);
        ~Function3DAdd();
        float3Data getFloat3Data(int x, int y, int z, int sx, int sy, int sz); //world coords

    private:
        std::vector<Function3D*> operands;
};

#endif // FUNCTION3DADD_HPP
