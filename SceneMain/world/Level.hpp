#ifndef LEVEL_HPP
#define LEVEL_HPP

#include<vector>

class Level
{
	public:
		int tx, ty, tz;
		std::vector<unsigned char> blocks;

		unsigned char getBlock(int x, int y, int z) const;
		void setBlock(int x, int y, int z, unsigned char block);
};

#endif // LEVEL_HPP
