#ifndef WORLDGENERATOR_HPP
#define WORLDGENERATOR_HPP
#include "commons.hpp"

class FunctionTerrain;
class TaskPool;
class Column;
class ColumnGenerator {
    public:
         ColumnGenerator(int seed);
        ~ColumnGenerator();

        void enqueueTask(vec2i column);
        void discardTasks();
        bool currentlyWorking(vec2i column);
        bool locked() const;
        void lock();
        void unlock();
        Column* pullDone();

    private:
        struct Comp {
                bool operator()(const vec2i& a, const vec2i& b) {
                    if(a.x != b.x) return a.x < b.x;
                    if(a.y != b.y) return a.y < b.y;
                    return false;
                }
        };

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
        std::map<vec2i, Column*, Comp> loaded;

        // Done and ready to be pulled into the game
        std::unique_lock<std::mutex> doneLock;
        std::mutex doneMutex;
        std::queue<Column*> done;

        // Currently being loaded/generated
        std::unique_lock<std::mutex> currentLock;
        std::mutex currentMutex;
        std::set<vec2i, Comp> current;

        // Jobs that try to load/generate a Column
        TaskPool* pool = nullptr;

        // Jobs that unload a Column
        TaskPool* killingPool = nullptr;
};

#endif // WORLDGENERATOR_HPP
