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
#include "Function3DHelix.hpp"
#include "TaskPool.hpp"

#define NWORKERS_GENERATING 2
#define NWORKERS_DECORATING 1
#define NWORKERS_BUILDING 1
#define NWORKERS_KILLING 1

ColumnGenerator::ColumnGenerator(int seed) {
    // Create locks
    loadedLock = std::unique_lock<std::mutex>(loadedMutex, std::defer_lock);
    doneLock = std::unique_lock<std::mutex>(doneMutex, std::defer_lock);

    // Create pools
    generatePool = new TaskPool(NWORKERS_GENERATING);
    decoratePool = new TaskPool(NWORKERS_DECORATING);
    buildPool = new TaskPool(NWORKERS_BUILDING);
    killPool = new TaskPool(NWORKERS_KILLING);

    // Create funtion tree for creation
    generator.seed(seed);
    Function3DSimplex* simplex31 = new Function3DSimplex(&generator,100,-70,70);
    Function3DSimplex* simplex32 = new Function3DSimplex(&generator,70,-50,50);
    Function3DSimplex* simplex33 = new Function3DSimplex(&generator,50,-30,30);
    Function3DSimplex* simplex34 = new Function3DSimplex(&generator,25,-20,20);
    Function3DSimplex* simplex35 = new Function3DSimplex(&generator,10,-5,5);
    Function3DAdd* add1 = new Function3DAdd({simplex31, simplex32, simplex33, simplex34, simplex35});
    Function3DYcoord* coord1 = new Function3DYcoord();
    Function2DConst* const1 = new Function2DConst(70);
    Function3DSub* sub1 = new Function3DSub(coord1,const1);
    Function3DSub* sub2 = new Function3DSub(add1,sub1);
    Function3DHelix* hel1 = new Function3DHelix(20,20,5,0,-20,0,5,25,40);
    Function3DHelix* hel3 = new Function3DHelix(5,10,5,0,-10,10,10,25,40);
    Function3DHelix* hel2 = new Function3DHelix(5,10,5,15,-10,10,10,25,40);
    Function3DHelix* hel4 = new Function3DHelix(20,20,5,45,-20,0,5,25,40);
    Function3DAdd* add2 = new Function3DAdd({sub2, hel1, hel2, hel3, hel4});
    FunctionTerrrainVolume* vol1 = new FunctionTerrrainVolume(add2, 2);
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
    generatePool->discard();
    delete generatePool;
    delete decoratePool;
    delete buildPool;
    delete killPool;
    delete entry;
    while(!done.empty()) {
        delete done.front();
        done.pop();
    }
}

void ColumnGenerator::queueLoad(vec2i colPos) {
    generatePool->enqueue([this, colPos]() {
        // Grab the job.
        {
            std::lock_guard<std::mutex> lock(loadedMutex);
            if(loaded.find(colPos) != loaded.end()) return;
            loaded.insert(std::pair<vec2i, ColumnData*>(colPos, new ColumnData()));
        }

        // Generate
        ID3Data raw = entry->getID3Data(colPos.x*CHUNKSIZE,0,colPos.y*CHUNKSIZE,CHUNKSIZE,CHUNKSIZE*GENERATIONHEIGHT,CHUNKSIZE);

        // Finished generating. Mark it as Raw.
        {
            std::lock_guard<std::mutex> lock(loadedMutex);
            VBE_ASSERT_SIMPLE(loaded.find(colPos) != loaded.end());
            ColumnData::State exp = ColumnData::Loading;
            bool done = loaded.at(colPos)->state.compare_exchange_strong(exp, ColumnData::Raw);
            VBE_ASSERT_SIMPLE(done);
            (void) done;
            loaded.at(colPos)->raw = new ID3Data(std::move(raw));
        }

        // Queue building (not decorating yet!)
        queueBuild(colPos);
    });
}
void ColumnGenerator::queueBuild(vec2i colPos) {
    buildPool->enqueue([this, colPos]() {
        // Grab the job for Building
        ID3Data* data = nullptr;
        {
            std::lock_guard<std::mutex> lock(loadedMutex);
            if(loaded.find(colPos) == loaded.end()) return;
            ColumnData::State exp = ColumnData::Raw;
            if(!loaded.at(colPos)->state.compare_exchange_strong(exp, ColumnData::Building)) {
                Log::message() << exp << Log::Flush;
                return;
            }
            data = loaded.at(colPos)->raw;
        }

        // Generate Column object
        Column* col = new Column(colPos.x,colPos.y);
        // Resize to hold all the chunks
        col->chunks.resize(GENERATIONHEIGHT,nullptr);
        // Traverse all and keep an eye on whether it's full or not
        for(int i = 0; i < GENERATIONHEIGHT; ++i) {
            col->chunks[i] = new Chunk(colPos.x,i,colPos.y);
            bool full = false;
            for(int x = 0; x < CHUNKSIZE; ++x)
                for(int y = 0; y < CHUNKSIZE; ++y)
                    for(int z = 0; z < CHUNKSIZE; ++z) {
                        if((*data)[x][i*CHUNKSIZE+y][z] != 0) full = true;
                        col->chunks[i]->cubes[x][y][z] = (*data)[x][i*CHUNKSIZE+y][z];
                    }
            // If chunk is empty, delete it
            if(!full) {
                delete col->chunks[i];
                col->chunks[i] = nullptr;
            }
        }
        // Shrink chunk vector
        for(int i = GENERATIONHEIGHT-1; i >= 0; --i) {
            if(col->chunks[i] == nullptr) col->chunks.resize(i);
            else break;
        }

        // Finished building. Mark it as built.
        {
            std::lock_guard<std::mutex> lock(loadedMutex);
            VBE_ASSERT_SIMPLE(loaded.find(colPos) != loaded.end());
            ColumnData::State exp = ColumnData::Building;
            bool done = loaded.at(colPos)->state.compare_exchange_strong(exp, ColumnData::Built);
            VBE_ASSERT_SIMPLE(done);
            (void) done;
            loaded.at(colPos)->col = col;
        }

        // Queue for collection
        {
            std::lock_guard<std::mutex> lock(doneMutex);
            done.push(col);
        }
    });
}

bool ColumnGenerator::locked() const {
    return (loadedLock.owns_lock() && doneLock.owns_lock());
}

void ColumnGenerator::lock() {
    VBE_ASSERT_SIMPLE(!locked());
    loadedLock.lock();
    doneLock.lock();
}

void ColumnGenerator::unlock() {
    VBE_ASSERT_SIMPLE(locked());
    loadedLock.unlock();
    doneLock.unlock();
}

void ColumnGenerator::discardTasks() {
    generatePool->discard();
}

Column* ColumnGenerator::pullDone() {
    std::unique_lock<std::mutex> l;
    if(!locked()) l = std::unique_lock<std::mutex>(doneMutex);

    if (done.empty()) return nullptr;

    Column* result = done.front();
    done.pop();

    return result;
}
