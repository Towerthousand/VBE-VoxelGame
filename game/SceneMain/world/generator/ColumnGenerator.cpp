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

void ColumnGenerator::update() {
    VBE_ASSERT_SIMPLE(locked());
    discardKillTasks();
    std::list<vec2i> toDelete;
    for(const std::pair<vec2i, ColumnData*>& kv : loaded) {
        if(!inPlayerArea(kv.first) && kv.second->canDelete()) {
            queueDelete(kv.first);
            continue;
        }
        switch(kv.second->state) {
            case ColumnData::Loading:
            case ColumnData::Building:
            case ColumnData::Deleting:
            case ColumnData::Built:
                break;
            case ColumnData::Raw:
                queueBuild(kv.first);
                break;
            case ColumnData::Deleted:
                toDelete.push_back(kv.first);
                break;
            default:
                break;
        }
    }
    for(const vec2i& p : toDelete) {
        ColumnData* cp = loaded.at(p);
        loaded.erase(p);
        if(cp->col != nullptr) delete cp->col;
        if(cp->raw != nullptr) delete cp->raw;
        delete cp;
    }
}

void ColumnGenerator::queueLoad(vec2i colPos) {
    generatePool->enqueue([this, colPos]() {
        ColumnData* colData = nullptr;

        // Grab the job.
        {
            std::lock_guard<std::mutex> lock(loadedMutex);
            if(loaded.find(colPos) != loaded.end()) return;
            colData = new ColumnData();
            ++colData->refCount;
            loaded.insert(std::pair<vec2i, ColumnData*>(colPos, colData));
        }

        // Generate
        ID3Data raw = entry->getID3Data(
            colPos.x*CHUNKSIZE,
            0,
            colPos.y*CHUNKSIZE,
            CHUNKSIZE,
            CHUNKSIZE*GENERATIONHEIGHT,
            CHUNKSIZE
        );

        // Finished generating. Mark it as Raw.
        {
            std::lock_guard<std::mutex> lock(loadedMutex);
            VBE_ASSERT_SIMPLE(loaded.find(colPos) != loaded.end());
            VBE_ASSERT_SIMPLE(loaded.at(colPos) == colData);
            VBE_ASSERT_SIMPLE(colData->state == ColumnData::Loading);
            colData->state = ColumnData::Raw;
            colData->raw = new ID3Data(std::move(raw));
            --colData->refCount;
        }
    });
}

void ColumnGenerator::queueBuild(vec2i colPos) {
    buildPool->enqueue([this, colPos]() {
        // Grab the job for Building
        ColumnData* colData = nullptr;
        {
            std::lock_guard<std::mutex> lock(loadedMutex);
            auto it = loaded.find(colPos);
            if(it == loaded.end()) return;
            colData = it->second;
            if(colData->state != ColumnData::Raw) return;
            colData->state = ColumnData::Building;
            colData->refCount++;
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
                        if((*colData->raw)[x][i*CHUNKSIZE+y][z] != 0) full = true;
                        col->chunks[i]->cubes[x][y][z] = (*colData->raw)[x][i*CHUNKSIZE+y][z];
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
            VBE_ASSERT_SIMPLE(loaded.at(colPos) == colData);
            VBE_ASSERT_SIMPLE(colData->state == ColumnData::Building);
            colData->state = ColumnData::Built;
            colData->col = col;
            --colData->refCount; // for this thread
            ++colData->refCount; // for the player
        }

        // Queue for collection
        {
            std::lock_guard<std::mutex> lock(doneMutex);
            done.push(col);
        }
    });
}

void ColumnGenerator::queueDelete(vec2i colPos) {
    buildPool->enqueue([this, colPos]() {
        // Grab the job for Building
        ColumnData* colData = nullptr;
        {
            std::lock_guard<std::mutex> lock(loadedMutex);
            auto it = loaded.find(colPos);
            if(it == loaded.end()) return;
            colData = it->second;
            if(!colData->canDelete()) return;
            colData->state = ColumnData::Deleting;
        }

        // Destroy me like one of your french girls
        // (save to disk, etc etc)

        // Finished Deleting. Mark it as deleted
        // Main thread will actually call the delete operator.
        {
            std::lock_guard<std::mutex> lock(loadedMutex);
            VBE_ASSERT_SIMPLE(loaded.find(colPos) != loaded.end());
            VBE_ASSERT_SIMPLE(loaded.at(colPos) == colData);
            VBE_ASSERT_SIMPLE(colData->state == ColumnData::Deleting);
            colData->state = ColumnData::Deleted;
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

void ColumnGenerator::discardGenerateTasks() {
    generatePool->discard();
}

void ColumnGenerator::discardKillTasks() {
    killPool->discard();
}

Column* ColumnGenerator::pullDone() {
    std::unique_lock<std::mutex> l;
    if(!locked()) l = std::unique_lock<std::mutex>(doneMutex);

    if (done.empty()) return nullptr;

    Column* result = done.front();
    done.pop();

    return result;
}

void ColumnGenerator::unloadColumn(Column* col) {
    std::unique_lock<std::mutex> l;
    if(!locked()) l = std::unique_lock<std::mutex>(loadedMutex);

    vec2i colPos = {col->getX(), col->getZ()};
    VBE_ASSERT_SIMPLE(loaded.find(colPos) != loaded.end());
    VBE_ASSERT_SIMPLE(loaded.at(colPos)->col == col);
    VBE_ASSERT_SIMPLE(loaded.at(colPos)->state == ColumnData::Built);
    loaded.at(colPos)->refCount--;
}

bool ColumnGenerator::inPlayerArea(const vec2i& colPos) const {
    return colPos.x >= relevantMin.x &&
           colPos.x <= relevantMax.x &&
           colPos.y >= relevantMin.y &&
           colPos.y <= relevantMax.y;
}

void ColumnGenerator::setRelevantArea(const vec2i& min, const vec2i&max) {
    VBE_ASSERT_SIMPLE(min.x <= max.x && min.y <= max.y);
    relevantMin = min;
    relevantMax = max;
}
