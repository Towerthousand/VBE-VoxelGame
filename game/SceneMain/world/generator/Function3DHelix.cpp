#include "Function3DHelix.hpp"

Function3DHelix::Function3DHelix(float period, float width, float range, float offset, float tiling) :
	period(period),
	width(width),
	range(range),
	offset(offset),
	tiling(tiling)
	{
}

Function3DHelix::~Function3DHelix() {
}

float Function3DHelix::helix(float x, float y, float z) {
	float cx = glm::mod(x,width*tiling) - width - glm::cos((y+offset)/period) * width;
	float cz = glm::mod(z,width*tiling) - width - glm::sin((y+offset)/period) * width;
	if(cx > 0 && cz > 0 && cx < range && cz < range)
		return 10000;
	return 0;
}

float3Data Function3DHelix::getFloat3Data(int x, int y, int z, int sx, int sy, int sz) {
	float3Data result(sx,float2Data(sy,float1Data(sz,0.0)));
	for(int localX = 0; localX < sx; ++localX)
		for(int localY = 0; localY < sy; ++localY)
			for(int localZ = 0; localZ < sz; ++localZ)
				result[localX][localY][localZ] = helix(x+localX, y+localY, z+localZ);
	return result;
}

