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
		Column* pullDone();

	private:
		struct Comp{
				bool operator()(const vec2i& a, const vec2i& b) {
					if(a.x != b.x) return a.x < b.x;
					if(a.y != b.y) return a.y < b.y;
					return false;
				}
				bool operator()(const std::pair<float,vec2i>& a, const std::pair<float,vec2i>& b) {
					if(a.first != b.first) return a.first < b.first;
					if(a.second.x != b.second.x) return a.second.x < b.second.x;
					if(a.second.y != b.second.y) return a.second.y < b.second.y;
					return false;
				}
		};


		std::mutex              currentMutex;
		std::set<vec2i, Comp>   current;
		std::mutex          doneMutex;
		std::queue<Column*> done;
		std::mt19937 generator;
		FunctionTerrain* entry;
		TaskPool* pool;
};

#endif // WORLDGENERATOR_HPP
