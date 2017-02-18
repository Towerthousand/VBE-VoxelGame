#include "ColumnGenerator.hpp"
#include "../Column.hpp"
#include "../Chunk.hpp"
#include "TaskPool.hpp"
#include "DecTrees.hpp"

#define NWORKERS_GENERATING 4
#define NWORKERS_DECORATING 1
#define NWORKERS_BUILDING 1
#define NWORKERS_KILLING 1
#define BIOME_RADIUS 16

ColumnGenerator::ColumnGenerator(int seed) {
    // Create locks
    loadedLock = std::unique_lock<std::mutex>(loadedMutex, std::defer_lock);
    doneLock = std::unique_lock<std::mutex>(doneMutex, std::defer_lock);

    // Create pools
    generatePool = new TaskPool(NWORKERS_GENERATING);
    decoratePool = new TaskPool(NWORKERS_DECORATING);
    buildPool = new TaskPool(NWORKERS_BUILDING);
    killPool = new TaskPool(NWORKERS_KILLING);

    // Create function tree for creation
    generator.seed(seed);
    Function2DSimplex* simplex21 = new Function2DSimplex(&generator, SIMPLEX_LOW, SIMPLEX_HIGH, 100.0f);
    FunctionTerrainHeightmap* terrain1 = new FunctionTerrainHeightmap(simplex21, MAIN_TERRAIN);
    FunctionTerrainOverlay* over1 = new FunctionTerrainOverlay(terrain1,1,2,4);
    FunctionTerrainOverlay* over2 = new FunctionTerrainOverlay(over1,3,1,1);
    terrainEntry = over2;

    // Create function tree for biomes
    FunctionBiome* b = new BiomeConst(&generator, OCEAN);
    b = new BiomeReplace(&generator, b, BiomeSet({OCEAN}), PLAINS, 2, false);
    b = new BiomeZoom(&generator, b, true);
    b = new BiomeSmooth(&generator, b);
    b = new BiomeZoom(&generator, b, true);
    b = new BiomeSmooth(&generator, b);
    b = new BiomeZoom(&generator, b, true);
    b = new BiomeSmooth(&generator, b);
    b = new BiomeZoom(&generator, b, true);
    b = new BiomeSmooth(&generator, b);
    b = new BiomeZoom(&generator, b, true);
    b = new BiomeSmooth(&generator, b);
    b = new BiomeZoom(&generator, b, true);
    b = new BiomeSmooth(&generator, b);
    b = new BiomeZoom(&generator, b, true);
    b = new BiomeSmooth(&generator, b);
    b = new BiomeSmooth(&generator, b);
    biomeEntry = b;

    //// Testing, flat grass at y=70
    //if(0) {
    //    Function3DYcoord* c = new Function3DYcoord();
    //    Function2DConst* co = new Function2DConst(70);
    //    Function3DSub* s = new Function3DSub(co, c);
    //    FunctionTerrainVolume* v1 = new FunctionTerrainVolume(s, 2);
    //    FunctionTerrainOverlay* o1 = new FunctionTerrainOverlay(v1,1,2,4);
    //    FunctionTerrainOverlay* o2 = new FunctionTerrainOverlay(o1,3,1,1);
    //    terrainEntry = o2;
    //}

    // Create decorators
    decorators.push_back(new DecTrees(&generator, TREE_MIN_GRID, TREE_MAX_GRID, TREE_CUTOFF, TREE_DROP_CHANCE));
}

ColumnGenerator::~ColumnGenerator() {
    generatePool->discard();
    delete generatePool;
    delete decoratePool;
    delete buildPool;
    delete killPool;
    delete terrainEntry;
    delete biomeEntry;
    while(!done.empty()) {
        delete done.front();
        done.pop();
    }
    for(auto d : decorators)
        delete d;
}

void ColumnGenerator::update() {
    VBE_ASSERT_SIMPLE(locked());
    killPool->discard();
    buildPool->discard();
    decoratePool->discard();
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
                queueDecorate(kv.first);
                break;
            case ColumnData::Decorated:
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
        if(cp->raw != nullptr) delete[] cp->raw;
        if(cp->biomes != nullptr) delete[] cp->biomes;
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
            colData->pos = colPos;
            ++colData->refCount;
            loaded.insert(std::pair<vec2i, ColumnData*>(colPos, colData));
        }

        // Generate biomes
        colData->biomes = new Biome[BIOME_MATRIX_SIZE*BIOME_MATRIX_SIZE];
        std::vector<int> biomevec = biomeEntry->getBiomeData(colPos.x*CHUNKSIZE-BIOME_MATRIX_MARGIN, colPos.y*CHUNKSIZE-BIOME_MATRIX_MARGIN, BIOME_MATRIX_SIZE, BIOME_MATRIX_SIZE);
        for(int x = 0; x < BIOME_MATRIX_SIZE; ++x)
            for(int z = 0; z < BIOME_MATRIX_SIZE; ++z)
                colData->biomes[x*BIOME_MATRIX_SIZE+z] = Biome(biomevec[z*BIOME_MATRIX_SIZE+x]);

        static thread_local int* biomeSums = new int[BIOME_MATRIX_SIZE*BIOME_MATRIX_SIZE*NUM_BIOMES];

        // Two-pass 2D biome cumsum
        for(int b = 0; b < NUM_BIOMES; ++b)
            for(int x = 0; x < BIOME_MATRIX_SIZE; ++x)
                for(int z = 0; z < BIOME_MATRIX_SIZE; ++z) {
                    biomeSums[b*BIOME_MATRIX_SIZE*BIOME_MATRIX_SIZE+x*BIOME_MATRIX_SIZE+z] = colData->biomes[x*BIOME_MATRIX_SIZE+z] == Biome(b)? 1 : 0;
                    if(x > 0) biomeSums[b*BIOME_MATRIX_SIZE*BIOME_MATRIX_SIZE+x*BIOME_MATRIX_SIZE+z] += biomeSums[b*BIOME_MATRIX_SIZE*BIOME_MATRIX_SIZE+(x-1)*BIOME_MATRIX_SIZE+z];
                }
        for(int b = 0; b < NUM_BIOMES; ++b)
            for(int x = 0; x < BIOME_MATRIX_SIZE; ++x)
                for(int z = 1; z < BIOME_MATRIX_SIZE; ++z)
                    biomeSums[b*BIOME_MATRIX_SIZE*BIOME_MATRIX_SIZE+x*BIOME_MATRIX_SIZE+z] += biomeSums[b*BIOME_MATRIX_SIZE*BIOME_MATRIX_SIZE+x*BIOME_MATRIX_SIZE+(z-1)];

        // Generate cube data
        unsigned int* raw =  new unsigned int[CHUNKSIZE*CHUNKSIZE*CHUNKSIZE*GENERATIONHEIGHT];
        for(int x = 0; x < CHUNKSIZE; ++x)
            for(int z = 0; z < CHUNKSIZE; ++z) {
                // Calculate average
                static thread_local std::valarray<float> par;
                par.resize(NUM_BIOME_PARAMS, 0.0f);
                int total = 0;
                for(int b = 0; b < NUM_BIOMES; ++b) {
                    // + Top right
                    int sum = biomeSums[b*BIOME_MATRIX_SIZE*BIOME_MATRIX_SIZE+(x+BIOME_RADIUS*2)*BIOME_MATRIX_SIZE+(z+BIOME_RADIUS*2)];
                    // - Bottom right
                    sum -= biomeSums[b*BIOME_MATRIX_SIZE*BIOME_MATRIX_SIZE+(x+BIOME_RADIUS*2)*BIOME_MATRIX_SIZE+z];
                    // - Top left
                    sum -= biomeSums[b*BIOME_MATRIX_SIZE*BIOME_MATRIX_SIZE+x*BIOME_MATRIX_SIZE+(z+BIOME_RADIUS*2)];
                    // + Bottom left
                    sum += biomeSums[b*BIOME_MATRIX_SIZE*BIOME_MATRIX_SIZE+x*BIOME_MATRIX_SIZE+z];
                    total += sum;
                    par += BIOME_PARAMS[b]*float(sum);
                }
                par /= total;
                // Generate terrain
                terrainEntry->fillData(
                    colPos.x*CHUNKSIZE+x,
                    colPos.y*CHUNKSIZE+z,
                    &raw[x*CHUNKSIZE*CHUNKSIZE*GENERATIONHEIGHT + z*CHUNKSIZE*GENERATIONHEIGHT],
                    &par,
                    &BIOME_INT_PARAMS[colData->biomes[(BIOME_MATRIX_MARGIN+x)*BIOME_MATRIX_SIZE+z]]
                );
            }

        // Finished generating. Mark it as Raw.
        {
            std::lock_guard<std::mutex> lock(loadedMutex);
            VBE_ASSERT_SIMPLE(loaded.find(colPos) != loaded.end());
            VBE_ASSERT_SIMPLE(loaded.at(colPos) == colData);
            VBE_ASSERT_SIMPLE(colData->state == ColumnData::Loading);
            colData->state = ColumnData::Raw;
            colData->raw = raw;
            --colData->refCount;
        }
    });
}

void ColumnGenerator::queueDecorate(vec2i colPos) {
    buildPool->enqueue([this, colPos]() {
        // Grab the job for Decorating
        ColumnData* colData = nullptr;
        bool onMyOwn = false;
        {
            std::lock_guard<std::mutex> lock(loadedMutex);
            auto it = loaded.find(colPos);
            if(it == loaded.end()) return;
            colData = it->second;
            if(colData->state != ColumnData::Raw) return;
            for(int x = -1; x <= 1; ++x)
                for(int y = -1; y <= 1; ++y) {
                    auto it2 = loaded.find(colPos+vec2i(x, y));
                    // This chunk is in the edge of the currently loaded world.
                    // Wait for neighboring chunks to be loaded/created
                    if(it2 == loaded.end())
                        return;
                    // This chunk has a neighbor who is being created or destroyed
                    if(it2->second->state < ColumnData::Raw ||
                       it2->second->state > ColumnData::Built)
                        return;
                    // Special case (shouldn't happen unless you tamper with weird
                    // savegames or have manually deleted chunks. What happens here
                    // is that a chunk is being loaded from scratch with a fully loaded
                    // (and built) chunk by it's side. Since decorating is a chunk-to-chunk
                    // process (each chunk needs the neighboring input to proceed), if we just
                    // go on we'll be losing the potential decorations from the already-built
                    // chunk. Hence, we must do it differently: Generate this chunk by generating
                    // all 8 surrounding chunks and decorating them, then apply this chunk's
                    // decorations to the real neighbors who are too being decorated (skip the
                    // built ones)
                    if(it2->second->state > ColumnData::Decorated)
                        onMyOwn = true;
                }
            for(int x = -1; x <= 1; ++x)
                for(int y = -1; y <= 1; ++y)
                    ++loaded.at(colPos+vec2i(x, y))->refCount;
            colData->state = ColumnData::Decorating;
        }

        // Regenerating broken chunks is not supported yet. Try not to mess up your saves ;)
        VBE_ASSERT(!onMyOwn, "Unsupported standalone chunk regen");
        (void) onMyOwn;

        // Decorate here. You can add any stuff you whant by calling setDecorationRC on colData.
        // Decorations can be placed anywhere on the nearby world space: That is: from
        // [-16, 31] on x and z, [0, min(32767, CHUNKSIZE*GENERATIONHEIGHT)] on y. If these
        // decorations fall out of this chunk (outside of the [0, 15] range for x or z) they will
        // be pushed onto the corresponding chunk at the end of the decoration stage.

        // Testing decorations for spawn chunk
        if(colPos == vec2i(0,0)) {
            for(short i = -2; i < 2; ++i)
                for(short y = 0; y < 128; ++y) {
                    colData->setDecorationRC(i   , y,    8, 0, 5);
                    colData->setDecorationRC(8   , y,    i, 0, 5);
                    colData->setDecorationRC(16+i, y,    8, 0, 5);
                    colData->setDecorationRC(8   , y, 16+i, 0, 5);
                }
            for(short y = 0; y < 256; ++y)
                colData->setDecorationRC(8, y, 8, 0, 6);
        }

        for(Dec* d : decorators)
            d->decorate(colData);

        // Finished decorating. Mark it as decorated.
        {
            std::lock_guard<std::mutex> lock(loadedMutex);
            VBE_ASSERT_SIMPLE(loaded.find(colPos) != loaded.end());
            VBE_ASSERT_SIMPLE(loaded.at(colPos) == colData);
            VBE_ASSERT_SIMPLE(colData->state == ColumnData::Decorating);
            colData->state = ColumnData::Decorated;
            for(int x = -1; x <= 1; ++x)
                for(int y = -1; y <= 1; ++y) {
                    ColumnData* nd = loaded.at(colPos+vec2i(x, y));
                    --nd->refCount;
                    // If this is the current chunk or the chunk is already fully decorated, skip
                    if(nd == colData || nd->state > ColumnData::Decorated)
                        continue;
                    // Push all decorations onto neighboring chunks so that they can use them
                    // on their building step.
                    nd->decOut.insert(colData->decIn[x+1][y+1].begin(), colData->decIn[x+1][y+1].end());
                }
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
            if(colData->state != ColumnData::Decorated) return;
            for(int x = -1; x <= 1; ++x)
                for(int y = -1; y <= 1; ++y) {
                    auto it2 = loaded.find(colPos+vec2i(x, y));
                    if(it2 == loaded.end())
                        return;
                    if(it2->second->state < ColumnData::Decorated ||
                       it2->second->state > ColumnData::Built) {
                        return;
                    }
                }
            colData->state = ColumnData::Building;
            ++colData->refCount; // for the player
        }

        // Generate Column object
        Column* col = new Column(colPos.x,colPos.y);
        // Gather decorations
        colData->decOut.insert(colData->decIn[1][1].begin(), colData->decIn[1][1].end());
        // Resize to hold all the chunks
        col->chunks.resize(GENERATIONHEIGHT,nullptr);
        // Traverse all and keep an eye on whether it's full or not
        for(int i = 0; i < GENERATIONHEIGHT; ++i) {
            col->chunks[i] = new Chunk(colPos.x,i,colPos.y);
            bool full = false;
            for(int x = 0; x < CHUNKSIZE; ++x)
                for(int z = 0; z < CHUNKSIZE; ++z)
                    for(int y = 0; y < CHUNKSIZE; ++y) {
                        vec3us c = {x, i*CHUNKSIZE+y, z};
                        // Start off with generation data
                        unsigned int dest = colData->raw[c.x*CHUNKSIZE*CHUNKSIZE*GENERATIONHEIGHT + c.z*CHUNKSIZE*GENERATIONHEIGHT + c.y];
                        // Search for match
                        auto it = colData->decOut.lower_bound(std::make_pair(c, 0));
                        if(it != colData->decOut.end() && it->first.first == c) {
                            // Search for highest layer
                            while(it != colData->decOut.end() && it->first.first == c)
                               ++it;
                            --it;
                            // Only override Generation val if layer > 127 or air.
                            if(it->first.second >= 127 || dest == 0)
                                dest = it->second;
                        }
                        if(dest != 0) full = true;
                        col->chunks[i]->cubes[x][y][z] = dest;
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
            --colData->refCount; // for the thread
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
