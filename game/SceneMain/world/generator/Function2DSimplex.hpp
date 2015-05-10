#ifndef FUNCTION2DSIMPLEX_HPP
#define FUNCTION2DSIMPLEX_HPP
#include "Function2D.hpp"

class Function2DSimplex : public Function2D {
	public:
		Function2DSimplex(std::mt19937* generator, float scale, float min, float max);
		~Function2DSimplex();
		float2Data getFloat2Data(int x, int z, int sx, int sz); //world coords
	private:
		float valSimplex2D( const float x, const float y); //world coords, return is in [min,max]
		int fastfloor(const float x);
		float dot(const int* g, const float x, const float y);
		std::vector<int> perm;
		static const int grad3[12][3];
		std::mt19937* generator;

		float scale;
		float min;
		float max;
};
#endif // FUNCTION2DSIMPLEX_HPP
