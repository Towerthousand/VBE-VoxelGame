#ifndef NOISE2D_HPP
#define NOISE2D_HPP
#include "commons.hpp"

class Noise2D {
    public:
        Noise2D();
        float get(float x, float y) const;
        float octavedGet(float x, float y, unsigned int octaves) const;

    private:
        int fastfloor(const float x) const;
        float dot(const int* g, const float x, const float y) const;

        std::mt19937* generator;
        std::vector<int> perm;
        static const int grad3[12][3];
};

#endif // NOISE_HPP
