#ifndef NOISE3D_HPP
#define NOISE3D_HPP
#include "commons.hpp"

class Noise3D {
    public:
        Noise3D(std::mt19937* generator, float min, float max, float scale);
        ~Noise3D();

        float get(float x, float y, float z) const {
            return simplex(x/scale, y/scale, z/scale);
        }
        float octavedGet(float x, float y, float z, unsigned int octaves) const;

    private:
        float simplex(float x, float y, float z) const;

        int fastfloor(const float x) const;
        float dot(const int* g, const float x, const float y, const float z) const;

        const float min = 0.0f;
        const float max = 1.0f;
        const float scale = 1.0f;

        std::vector<int> perm;
        static const int grad3[12][3];
};

#endif // NOISE2D_HPP
