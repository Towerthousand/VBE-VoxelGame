#include "Function2DSimplex.hpp"

Function2DSimplex::Function2DSimplex(std::mt19937 *generator, float scale, float min, float max) :
	Function2D(), generator(generator), scale(scale), min(min), max(max){
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

Function2DSimplex::~Function2DSimplex() {
}

float2Data Function2DSimplex::getFloat2Data(int x, int z, int sx, int sz) { //world coords
	float2Data result(sx,float1Data(sz,0.0));
	for(int localX = 0; localX < sx; ++localX)
		for(int localZ = 0; localZ < sz; ++localZ) {
			vec2d pos((x+localX)/scale,(z+localZ)/scale);
			result[localX][localZ] = min + (max-min)*((glm::simplex(pos)+1)*0.5);
			//result[localX][localZ] = valSimplex2D(pos.x,pos.y); more precision, slower
		}
	return result;
}

float Function2DSimplex::valSimplex2D(const float x, const float y) {
	// Noise contributions from the three corners
	float n0, n1, n2;

	// Skew the input space to determine which simplex cell we're in
	float F2 = 0.5 * (sqrtf(3.0) - 1.0);
	// Hairy factor for 2D
	float s = (x + y) * F2;
	int i = fastfloor( x + s );
	int j = fastfloor( y + s );

	float G2 = (3.0 - sqrtf(3.0)) / 6.0;
	float t = (i + j) * G2;
	// Unskew the cell origin back to (x,y) space
	float X0 = i-t;
	float Y0 = j-t;
	// The x,y distances from the cell origin
	float x0 = x-X0;
	float y0 = y-Y0;

	// For the 2D case, the simplex shape is an equilateral triangle.
	// Determine which simplex we are in.
	int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
	if(x0>y0) {i1=1; j1=0;} // lower triangle, XY order: (0,0)->(1,0)->(1,1)
	else {i1=0; j1=1;} // upper triangle, YX order: (0,0)->(0,1)->(1,1)

	// A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
	// a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
	// c = (3-sqrt(3))/6
	float x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
	float y1 = y0 - j1 + G2;
	float x2 = x0 - 1.0 + 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
	float y2 = y0 - 1.0 + 2.0 * G2;

	// Work out the hashed gradient indices of the three simplex corners
	int ii = i & 255;
	int jj = j & 255;
	int gi0 = perm[ii+perm[jj]] % 12;
	int gi1 = perm[ii+i1+perm[jj+j1]] % 12;
	int gi2 = perm[ii+1+perm[jj+1]] % 12;

	// Calculate the contribution from the three corners
	float t0 = 0.5 - x0*x0-y0*y0;
	if(t0<0) n0 = 0.0;
	else {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0); // (x,y) of grad3 used for 2D gradient
	}

	float t1 = 0.5 - x1*x1-y1*y1;
	if(t1<0) n1 = 0.0;
	else {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
	}

	float t2 = 0.5 - x2*x2-y2*y2;
	if(t2<0) n2 = 0.0;
	else {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
	}

	// Add contributions from each corner to get the final noise value.
	// The result is scaled to return values in the interval [-1,1].
	float result = 70.0 * (n0 + n1 + n2);
	//scale the result
	result += 1; //in [0,2]
	result *= 0.5; // in [0,1]
	float difference = max-min;
	return (min + difference * result);
}

int Function2DSimplex::fastfloor( const float x ) { return x > 0 ? (int) x : (int) x - 1; }
float Function2DSimplex::dot( const int* g, const float x, const float y) { return g[0]*x + g[1]*y; }

const int Function2DSimplex::grad3[12][3] = {
	{1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
	{1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
	{0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1}
};
