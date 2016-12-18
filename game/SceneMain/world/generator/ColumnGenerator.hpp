#ifndef WORLDGENERATOR_HPP
#define WORLDGENERATOR_HPP
#include "commons.hpp"
#include "FunctionTerrain.hpp"
#include <atomic>

class FunctionTerrain;
class TaskPool;
class Column;
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

    private:
        void queueBuild(vec2i colPos);
        void queueDelete(vec2i colPos);
        void discardKillTasks();
        bool inPlayerArea(const vec2i& colPos) const;

        struct Comp {
                bool operator()(const vec2i& a, const vec2i& b) {
                    if(a.x != b.x) return a.x < b.x;
                    if(a.y != b.y) return a.y < b.y;
                    return false;
                }
        };

        struct ColumnData {
            // Raw column block data. May be empty if the column is
            // loaded into a Column object (so this->col != nullptr)
            ID3Data* raw = nullptr;
            // Finished column, with decorations and entities.
            // Will be nullptr if still being loaded, decorated, etc
            // in which case this->raw != nullptr
            Column* col = nullptr;

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

            State state = State::Loading;
            int refCount = 0;

            bool canDelete() {
                return refCount == 0 && (
                    state == Built ||
                    state == Decorated ||
                    state == Raw
                );
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
        FunctionTerrain* entry = nullptr;

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
