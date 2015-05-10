#ifndef FUNCTION3DSIMPLEX_HPP
#define FUNCTION3DSIMPLEX_HPP
#include "Function3D.hpp"

class Function3DSimplex : public Function3D {
	public:
		Function3DSimplex(std::mt19937* generator, float scale, float min, float max);
		~Function3DSimplex();
		float3Data getFloat3Data(int x, int y, int z, int sx, int sy, int sz); //world coords
	private:
		float valSimplex3D( const float x, const float y, const float z ); //world coords, return is in [min,max]
		int fastfloor(const float x);
		float dot(const int* g, const float x, const float y, const float z);
		std::vector<int> perm;
		static const int grad3[12][3];
		std::mt19937* generator;

		float scale;
		float min;
		float max;
};

#endif // FUNCTION3DSIMPLEX_HPP
