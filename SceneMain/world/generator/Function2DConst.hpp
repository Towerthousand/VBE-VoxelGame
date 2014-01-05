#ifndef FUNCTION3DCONSTANT_HPP
#define FUNCTION3DCONSTANT_HPP
#include "Function2D.hpp"

class Function2DConst : public Function2D {
	public:
		Function2DConst(float val);
		~Function2DConst();
		float2Data getFloat2Data(int x, int z, int sx, int sz); //world coords

	private:
		float val;
};

#endif // FUNCTION3DCONSTANT_HPP
