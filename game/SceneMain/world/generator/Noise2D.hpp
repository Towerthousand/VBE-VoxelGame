#ifndef NOISE2D_HPP
#define NOISE2D_HPP
#include "commons.hpp"

class Noise2D {
    public:
        Noise2D(std::mt19937* generator, float min, float max, float scale);
        ~Noise2D();

        inline float get(float x, float y) const {
            return simplex(x/scale, y/scale);
        }
        float octavedGet(float x, float y, unsigned int octaves) const;

    private:
        float simplex(float x, float y) const;

        int fastfloor(const float x) const;
        float dot(const int* g, const float x, const float y) const;

        const float min = 0.0f;
        const float max = 1.0f;
        const float scale = 1.0f;

        std::vector<int> perm;
        static const int grad3[12][3];
};

#endif // NOISE2D_HPP
