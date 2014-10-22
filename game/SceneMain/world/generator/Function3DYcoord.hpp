#ifndef FUNCTION3DYCOORD_HPP
#define FUNCTION3DYCOORD_HPP
#include "Function3D.hpp"

class Function3DYcoord : public Function3D {
	public:
		Function3DYcoord();
		~Function3DYcoord();
		float3Data getFloat3Data(int x, int y, int z, int sx, int sy, int sz); //world coords
};

#endif // FUNCTION3DYCOORD_HPP
