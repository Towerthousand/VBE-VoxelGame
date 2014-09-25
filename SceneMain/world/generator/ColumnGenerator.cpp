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

#define GENERATIONHEIGHT 512
#define NWORKERS 3

ColumnGenerator::ColumnGenerator(int seed) :
	TaskPool(NWORKERS),
	entry(NULL) {
	generator.seed(seed);
	Function3DSimplex* simplex31 = new Function3DSimplex(&generator,100,-700,7000);
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
	Function2DSimplex* simplex22 = new Function2DSimplex(&generator,100,0,130);
	FunctionTerrainHeightmap* terrain1 = new FunctionTerrainHeightmap(simplex21,2);
	FunctionTerrainHeightmap* terrain2 = new FunctionTerrainHeightmap(simplex22,2);
	FunctionTerrainJoin* join1 = new FunctionTerrainJoin(terrain2,terrain1);
	FunctionTerrainJoin* join2 = new FunctionTerrainJoin(vol1,join1);
	FunctionTerrainOverlay* over1 = new FunctionTerrainOverlay(join2,1,2,4);
	FunctionTerrainOverlay* over2 = new FunctionTerrainOverlay(over1,3,1,1);
	entry = over2;
}

ColumnGenerator::~ColumnGenerator() {
	delete entry;
}

void ColumnGenerator::enqueueTask(vec2i colPos) {
	enqueue([this, colPos]() {
		{
			std::unique_lock<std::mutex> lock(currentMutex);
			VBE_ASSERT(current.find(colPos) == current.end(), "You may not enqueue a colum that is already being worked on");
			current.insert(colPos);
		}

		Column* col = new Column(colPos.x,colPos.y);
		col->chunks.resize(GENERATIONHEIGHT,nullptr);
		ID3Data data = entry->getID3Data(colPos.x*CHUNKSIZE,0,colPos.y*CHUNKSIZE,CHUNKSIZE,CHUNKSIZE*GENERATIONHEIGHT,CHUNKSIZE);
		for(int i = 0; i < GENERATIONHEIGHT; ++i) {
			col->chunks[i] = new Chunk(colPos.x,i,colPos.y);
			bool full = false;
			for(int x = 0; x < CHUNKSIZE; ++x)
				for(int y = 0; y < CHUNKSIZE; ++y)
					for(int z = 0; z < CHUNKSIZE; ++z) {
						if(data[x][i*CHUNKSIZE+y][z] != 0) full = true;
						col->chunks[i]->cubes[x][y][z] = data[x][i*CHUNKSIZE+y][z];
					}
			if(!full) {
				delete col->chunks[i];
				col->chunks[i] = nullptr;
			}
		}
		for(int i = GENERATIONHEIGHT-1; i >= 0; --i) {
			if(col->chunks[i] == nullptr) col->chunks.resize(i);
			else break;
		}

		{
			std::unique_lock<std::mutex> lock(currentMutex);
			std::unique_lock<std::mutex> lock2(doneMutex);
			current.erase(colPos);
			done.push(col);
		}
	});
}

void ColumnGenerator::discardTasks() {
	discard();
}

bool ColumnGenerator::currentlyWorking(vec2i column) {
	std::unique_lock<std::mutex> lock(currentMutex);
	return current.find(column) != current.end();
}

Column* ColumnGenerator::pullDone() {
	std::unique_lock<std::mutex> lock(doneMutex);
	if (done.empty()) return nullptr;

	Column* result = done.front();
	done.pop();

	return result;
}
