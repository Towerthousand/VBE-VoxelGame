#include "ColumnGenerator.hpp"
#include "../Column.hpp"
#include "../Chunk.hpp"
#include "Function3DSimplex.hpp"
#include "Function3DSub.hpp"
#include "Function3DYcoord.hpp"
#include "Function3DAdd.hpp"
#include "Function2DConst.hpp"
#include "FunctionTerrrainVolume.hpp"
#include "FunctionTerrainOverlay.hpp"
#include "Function2DSimplex.hpp"
#include "FunctionTerrainHeightmap.hpp"
#include "FunctionTerrainJoin.hpp"

ColumnGenerator::ColumnGenerator(int seed) :
	entry(NULL) {
	generator.seed(seed);
	Function3DSimplex* simplex31 = new Function3DSimplex(&generator,100,-70,70);
	Function3DSimplex* simplex32 = new Function3DSimplex(&generator,70,-50,50);
	Function3DSimplex* simplex33 = new Function3DSimplex(&generator,50,-30,30);
	Function3DSimplex* simplex34 = new Function3DSimplex(&generator,25,-20,20);
	Function3DSimplex* simplex35 = new Function3DSimplex(&generator,10,-5,5);
	Function3DAdd* add1 = new Function3DAdd(simplex31,simplex32);
	Function3DAdd* add2 = new Function3DAdd(simplex34,simplex33);
	Function3DAdd* add3 = new Function3DAdd(add1,add2);
	Function3DAdd* add4 = new Function3DAdd(add3,simplex35);
	Function3DYcoord* coord1 = new Function3DYcoord();
	Function2DConst* const1 = new Function2DConst(70);
	Function3DSub* sub1 = new Function3DSub(coord1,const1);
	Function3DSub* sub2 = new Function3DSub(add4,sub1);
	FunctionTerrrainVolume* vol1 = new FunctionTerrrainVolume(sub2,2);
	Function2DSimplex* simplex21 = new Function2DSimplex(&generator,50,90,95);
	FunctionTerrainHeightmap* terrain1 = new FunctionTerrainHeightmap(simplex21,2);
	FunctionTerrainJoin* join1 = new FunctionTerrainJoin(vol1,terrain1);
	FunctionTerrainOverlay* over1 = new FunctionTerrainOverlay(join1,1,2,4);
	FunctionTerrainOverlay* over2 = new FunctionTerrainOverlay(over1,3,1,1);
	entry = over2;
}

ColumnGenerator::~ColumnGenerator() {
	delete entry; //will delete all child node functions recursively into the function tree
}

Column* ColumnGenerator::getColumn(int x, int z) {
	Column* col = new Column(x,z);
	col->chunks.resize(16,nullptr);
	ID3Data data = entry->getID3Data(x*CHUNKSIZE,0,z*CHUNKSIZE,CHUNKSIZE,CHUNKSIZE*16,CHUNKSIZE);
	for(int i = 0; i < 16; ++i) {
		col->chunks[i] = new Chunk(x,i,z);
		bool full = false;
		for(int x = 0; x < CHUNKSIZE; ++x)
			for(int y = 0; y < CHUNKSIZE; ++y)
				for(int z = 0; z < CHUNKSIZE; ++z) {
					if(data[x][i*CHUNKSIZE+y][z] != 0)
						full = true;
					col->chunks[i]->cubes[x][y][z] = data[x][i*CHUNKSIZE+y][z];
				}
		if(!full) {
			delete col->chunks[i];
			col->chunks[i] = nullptr;
		}
	}
	for(int i = 15; i >= 0; --i) {
		if(col->chunks[i] == nullptr)
			col->chunks.resize(i);
		else
			break;
	}
	return col;
}
