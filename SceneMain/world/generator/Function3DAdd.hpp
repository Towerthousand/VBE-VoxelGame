#ifndef FUNCTION3DADD_HPP
#define FUNCTION3DADD_HPP
#include "Function3D.hpp"

class Function3DAdd : public Function3D {
	public:
		Function3DAdd(Function3D* A, Function3D* B);
		~Function3DAdd();
		float3Data getFloat3Data(int x, int y, int z, int sx, int sy, int sz); //world coords

	private:
		Function3D* funcA;
		Function3D* funcB;
};

#endif // FUNCTION3DADD_HPP
