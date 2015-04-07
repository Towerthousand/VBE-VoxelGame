#ifndef FUNCTION3DHELIX_HPP
#define FUNCTION3DHELIX_HPP
#include "Function3D.hpp"

class Function3DHelix : public Function3D {
	public:
		Function3DHelix(float period, float width, float range, float offset, float tiling);
		~Function3DHelix();
		float3Data getFloat3Data(int x, int y, int z, int sx, int sy, int sz); //world coords

	private:
		float helix(float x, float y, float z);

		float period = 5;
		float width = 10;
		float range = 5;
		float offset = 0;
		float tiling = 5;
};

#endif // FUNCTION3DHELIX_HPP
