#ifndef WORLDGENERATOR_HPP
#define WORLDGENERATOR_HPP
#include "commons.hpp"
#include "terrainFunctions.hpp"
#include "biomeFunctions.hpp"

#define BIOME_MATRIX_MARGIN 32
#define BIOME_MATRIX_SIZE (CHUNKSIZE+BIOME_MATRIX_MARGIN*2)

class TaskPool;
class Column;
class Dec;
class ColumnGenerator {
    public:
         ColumnGenerator(int seed);
        ~ColumnGenerator();

        void queueLoad(vec2i colPos);
        void discardGenerateTasks();
        bool locked() const;
        void lock();
        void unlock();
        void unloadColumn(Column* col);
        void setRelevantArea(const vec2i& min, const vec2i&max);
        Column* pullDone();
        void update();

        struct ColumnData {
            struct PairComp {
                    bool operator()(const std::pair<vec3us, unsigned char>& a, const std::pair<vec3us, unsigned char>& b) {
                        if(a.first.x != b.first.x) return a.first.x < b.first.x;
                        if(a.first.y != b.first.y) return a.first.y < b.first.y;
                        if(a.first.z != b.first.z) return a.first.z < b.first.z;
                        if(a.second != b.second) return a.second < b.second;
                        return false;
                    }
            };

            enum State {
                Loading = 0,
                Raw,
                Decorating,
                Decorated,
                Building,
                Built,
                Deleting,
                Deleted
            };

            bool canDelete() const {
                return refCount == 0 && (
                    state == Built ||
                    state == Raw
                );
            }

            inline void setDecorationWC(vec3i v, unsigned char layer, unsigned int val) {
                setDecorationRC(v.x - pos.x*CHUNKSIZE, v.y, v.z - pos.y*CHUNKSIZE, layer, val);
            }

            inline void setDecorationWC(int x, int y, int z, unsigned char layer, unsigned int val) {
                setDecorationRC(x - pos.x*CHUNKSIZE, y, z - pos.y*CHUNKSIZE, layer, val);
            }

            inline void setDecorationRC(vec3s v, unsigned char layer, unsigned int val) {
                setDecorationRC(v.x, v.y, v.z, layer, val);
            }

            void setDecorationRC(short x, short y, short z, unsigned char layer, unsigned int val) {
                int x1 = (x+16) >> CHUNKSIZE_POW2;
                int z1 = (z+16) >> CHUNKSIZE_POW2;
                VBE_ASSERT_SIMPLE(z1 >= 0 && z1 <= 2 && z1 >= 0 && z1 <= 2);
                VBE_ASSERT_SIMPLE(y >= 0);
                auto r = decIn[x1][z1].insert(
                        std::make_pair(
                            std::make_pair(
                                vec3us(x & CHUNKSIZE_MASK, y, z & CHUNKSIZE_MASK),
                                layer
                            ),
                            val
                        )
                    );
                (void) r;
            }

            typedef std::map<std::pair<vec3us, unsigned char>, unsigned int, PairComp> ChunkDecoration;

            // Raw column block data. May be empty if the column is
            // loaded into a Column object (so this->col != nullptr)
            const unsigned int* raw = nullptr;
            // Raw biome data. Idem.
            Biome* biomes = nullptr;
            // Finished column, with decorations and entities.
            // Will be nullptr if still being loaded, decorated, etc
            // in which case this->raw != nullptr
            Column* col = nullptr;
            ChunkDecoration decIn[3][3];
            ChunkDecoration decOut;
            State state = State::Loading;
            int refCount = 0;
            vec2i pos = {0, 0};
        };

    private:
        void queueBuild(vec2i colPos);
        void queueDecorate(vec2i colPos);
        void queueDelete(vec2i colPos);
        bool inPlayerArea(const vec2i& colPos) const;

        struct Comp {
                bool operator()(const vec2i& a, const vec2i& b) {
                    if(a.x != b.x) return a.x < b.x;
                    if(a.y != b.y) return a.y < b.y;
                    return false;
                }
        };


        // All deleteable chunks outside this range will be deleted
        vec2i relevantMin = {0, 0};
        vec2i relevantMax = {0, 0};

        //      ____   _   __ ______
        //     / __ \ / | / // ____/ Do you have a moment
        //    / /_/ //  |/ // / __   to talk about our
        //   / _, _// /|  // /_/ /   lord and savior,
        //  /_/ |_|/_/ |_/ \____/    RNG?
        //
        std::mt19937 generator;

        // Terrain generation entrypoint
        FunctionTerrain* terrainEntry = nullptr;
        // Terrain decorators
        std::vector<Dec*> decorators;
        // Biome generation entrypoint
        FunctionBiome* biomeEntry = nullptr;

        // All loaded Columns. May be missing decorations
        std::unique_lock<std::mutex> loadedLock;
        std::mutex loadedMutex;
        std::map<vec2i, ColumnData*, Comp> loaded;

        // Done and ready to be pulled into the game
        std::unique_lock<std::mutex> doneLock;
        std::mutex doneMutex;
        std::queue<Column*> done;

        // Jobs that try to load/generate a ColumnData
        TaskPool* generatePool = nullptr;

        // Jobs that decorate a newly generated ColumnData
        TaskPool* decoratePool = nullptr;

        // Jobs grab a finished ColumnData,
        // builds the Column* object if necessary and computes
        // the initial 3D model for all it's chunks
        TaskPool* buildPool= nullptr;

        // Jobs that unload a ColumnData
        TaskPool* killPool = nullptr;
};

#endif // WORLDGENERATOR_HPP
