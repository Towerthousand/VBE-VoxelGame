#ifndef FUNCTION3DHELIX_HPP
#define FUNCTION3DHELIX_HPP
#include "Function3D.hpp"

class Function3DHelix : public Function3D {
    public:
        Function3DHelix(float period, float width, float range, float offset, float yoffset, float zoffset, float tiling, float sin, float sin2);
        ~Function3DHelix();
        virtual void fillData(int x, int z, floatType* data, GenParams* params) override;

    private:
        float helix(float x, float y, float z);

        float period = 5;
        float width = 10;
        float range = 5;
        float offset = 0;
        float yoffset = 0;
        float zoffset = 0;
        float tiling = 5;
        float sin = 0;
        float sin2 = 60;
};

#endif // FUNCTION3DHELIX_HPP
