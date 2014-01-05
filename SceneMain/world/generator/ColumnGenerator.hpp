#ifndef WORLDGENERATOR_HPP
#define WORLDGENERATOR_HPP
#include "commons.hpp"

class FunctionTerrain;
class Column;
class ColumnGenerator {
	public:
		ColumnGenerator(int seed);
		~ColumnGenerator();

		Column* getColumn(int x, int z);

	private:
		std::mt19937 generator; //Mersenne twister with nice configuration
		FunctionTerrain* entry; //root function for the generation tree
};

#endif // WORLDGENERATOR_HPP
