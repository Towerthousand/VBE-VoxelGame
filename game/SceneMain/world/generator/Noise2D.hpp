#ifndef NOISE2D_HPP
#define NOISE2D_HPP
#include "commons.hpp"

class Noise2D {
    public:
        Noise2D(std::mt19937* generator, float min, float max, float scale);
        ~Noise2D();

        float get(float x, float y) const;
        float octavedGet(float x, float y, unsigned int octaves) const;

    private:
        int fastfloor(const float x) const;
        float dot(const int* g, const float x, const float y) const;

        float min = 0.0f;
        float max = 1.0f;
        mutable float scale = 1.0f;

        std::vector<int> perm;
        static const int grad3[12][3];
};

#endif // NOISE2D_HPP
