#include "GenParams.hpp"

GenParams operator+(const GenParams& a, const GenParams& b) {
    return {
        a.lo+b.lo,
        a.hi+b.hi,
        a.scale+b.scale
    };
}

GenParams operator-(const GenParams& a, const GenParams& b) {
    return {
        a.lo-b.lo,
        a.hi-b.hi,
        a.scale-b.scale
    };
}

GenParams operator*(const GenParams& a, const float& b) {
    return {
        a.lo*b,
        a.hi*b,
        a.scale*b
    };
}

GenParams operator/(const GenParams& a, const float& b) {
    return {
        a.lo/b,
        a.hi/b,
        a.scale/b
    };
}
