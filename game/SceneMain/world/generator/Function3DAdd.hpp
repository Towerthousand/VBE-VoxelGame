#ifndef FUNCTION3DADD_HPP
#define FUNCTION3DADD_HPP
#include "Function3D.hpp"

class Function3DAdd : public Function3D {
    public:
        Function3DAdd(const std::vector<Function3D*>& operands);
        ~Function3DAdd();
        virtual void fillData(int x, int z, floatType* data, GenParams* params) override;

    private:
        std::vector<Function3D*> operands;
};

#endif // FUNCTION3DADD_HPP
