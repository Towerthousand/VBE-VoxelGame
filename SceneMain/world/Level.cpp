#include "Level.hpp"

using namespace std;

unsigned char Level::getBlock(int x, int y, int z) const
{
	return x >= 0 && y >= 0 && z >= 0 && x < tx && y < ty && z < tz
			? blocks[(y * tz + z) * tx + x] : 0;
}

void Level::setBlock(int x, int y, int z, unsigned char block)
{
	if(x >= 0 && y >= 0 && z >= 0 && x < tx && y < ty && z < tz)
		blocks[(y * tz + z) * tx + x] = block;
}
