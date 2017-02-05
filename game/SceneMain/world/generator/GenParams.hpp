#ifndef GENPARAMS3D_HPP
#define GENPARAMS3D_HPP

enum Biome {
    OCEAN = 0,
    PLAINS,
    NUM_BIOMES
};

struct GenParams {
    float lo;
    float hi;
    float scale;

    GenParams& operator+=(const GenParams& p) {
        lo += p.lo;
        hi += p.hi;
        scale += p.scale;

        return *this;
    };

    GenParams& operator-=(const GenParams& p) {
        lo -= p.lo;
        hi -= p.hi;
        scale -= p.scale;

        return *this;
    };

    GenParams& operator*=(const float& f) {
        lo *= f;
        hi *= f;
        scale *= f;

        return *this;
    };

    GenParams& operator/=(const float& f) {
        lo /= f;
        hi /= f;
        scale /= f;

        return *this;
    };
};

GenParams operator+(const GenParams& a, const GenParams& b);
GenParams operator-(const GenParams& a, const GenParams& b);
GenParams operator*(const GenParams& a, const float& b);
GenParams operator/(const GenParams& a, const float& b);

#endif // GENPARAMS3D_HPP
