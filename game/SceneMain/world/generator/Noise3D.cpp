#include "Noise3D.hpp"

const int Noise3D::grad3[12][3] = {
    {1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
    {1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
    {0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1}
};

Noise3D::Noise3D(std::mt19937* generator) {
    //generate permutation
    std::uniform_int_distribution<int> distribution(0,255);
    perm.resize(256);
    for (int i = 0; i < 256; i++)
        perm[i] = i;
    for (int i = 0; i < 256; i++) {
        int j = distribution(*generator);
        //Swap perm[i] and perm[j]
        int aux = perm[i];
        perm[i] = perm[j];
        perm[j] = aux;
    }
    perm.resize(512);
    for (int i = 0; i < 256; i++)
        perm[i+256] = perm[i];
}

Noise3D::~Noise3D() {
}

float Noise3D::get(float x, float y, float z) const {
    float n0, n1, n2, n3; // Noise contributions from the four corners

    // Skew the input space to determine which simplex cell we're in
    float F3 = 1.0f/3.0f;
    float s = (x+y+z)*F3; // Very nice and simple skew factor for 3D
    int i = fastfloor(x+s);
    int j = fastfloor(y+s);
    int k = fastfloor(z+s);

    float G3 = 1.0f/6.0f; // Very nice and simple unskew factor, too
    float t = (i+j+k)*G3;
    float X0 = i-t; // Unskew the cell origin back to (x,y,z) space
    float Y0 = j-t;
    float Z0 = k-t;
    float x0 = x-X0; // The x,y,z distances from the cell origin
    float y0 = y-Y0;
    float z0 = z-Z0;

    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords

    if(x0>=y0) {
        if(y0>=z0) { i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; } // X Y Z order
        else if(x0>=z0) { i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; } // X Z Y order
        else { i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; } // Z X Y order
    }
    else { // x0<y0
        if(y0<z0) { i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; } // Z Y X order
        else if(x0<z0) { i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; } // Y Z X order
        else { i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; } // Y X Z order
    }

    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.
    float x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
    float y1 = y0 - j1 + G3;
    float z1 = z0 - k1 + G3;
    float x2 = x0 - i2 + 2.0*G3; // Offsets for third corner in (x,y,z) coords
    float y2 = y0 - j2 + 2.0*G3;
    float z2 = z0 - k2 + 2.0*G3;
    float x3 = x0 - 1.0 + 3.0*G3; // Offsets for last corner in (x,y,z) coords
    float y3 = y0 - 1.0 + 3.0*G3;
    float z3 = z0 - 1.0 + 3.0*G3;

    // Work out the hashed gradient indices of the four simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    int gi0 = perm[ii+perm[jj+perm[kk]]] % 12;
    int gi1 = perm[ii+i1+perm[jj+j1+perm[kk+k1]]] % 12;
    int gi2 = perm[ii+i2+perm[jj+j2+perm[kk+k2]]] % 12;
    int gi3 = perm[ii+1+perm[jj+1+perm[kk+1]]] % 12;

    // Calculate the contribution from the four corners
    float t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
    if(t0<0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
    }

    float t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
    if(t1<0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
    }

    float t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
    if(t2<0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
    }

    float t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
    if(t3<0) n3 = 0.0;
    else {
        t3 *= t3;
        n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    float result = 32.0*(n0 + n1 + n2 + n3);
    //scale the result
    result += 1; //in [0,2]
    result *= 0.5; // in [0,1]
    return result;
}

float Noise3D::octavedGet(float x, float y, float z, unsigned int octaves) const {
    float currScale = 1.0f;
    float val = 0.0f;
    float numParts = float(1 << octaves) - 1;
    for(unsigned int i = 0; i < octaves; ++i) {
        float importance = float(1 << (octaves-i-1))/numParts;
        val += get(x/currScale, y/currScale, z/currScale)*importance;
        currScale *= 0.5f;
    }
    return val;
}

int Noise3D::fastfloor( const float x ) const { return x > 0 ? (int) x : (int) x - 1; }
float Noise3D::dot( const int* g, const float x, const float y, const float z ) const { return g[0]*x + g[1]*y + g[2]*z; }
