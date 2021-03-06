#ifndef NOISE3D_HPP
#define NOISE3D_HPP
#include "commons.hpp"

class Noise3D {
    public:
        Noise3D(std::mt19937* generator);
        ~Noise3D();

        float get(float x, float y, float z) const;
        float octavedGet(float x, float y, float z, unsigned int octaves) const;

    private:
        int fastfloor(const float x) const;
        float dot(const int* g, const float x, const float y, const float z) const;

        std::vector<int> perm;
        static const int grad3[12][3];
};

#endif // NOISE2D_HPP
