#ifndef WORLD_HPP
#define WORLD_HPP
#include "commons.hpp"
#include "Cube.hpp"
#include "generator/ColumnGenerator.hpp"

class Column;
class Camera;
class DeferredContainer;
class World : public GameObject {
	public:
		World();
		~World();

		void update(float deltaTime);
		void draw() const;

		bool outOfBounds(int x, int y, int z) const;
		Cube getCube(int x, int y, int z) const;
		Column* getColumn(int x, int y, int z) const;
		Camera* getCamera() const;

		void setCubeLight(int x, int y, int z, unsigned char light);
		void setCubeID(int x, int y, int z, unsigned char ID);
	private:
		
		struct LightJob {
			LightJob(const vec3i& pos, const vec3i& radius) : pos(pos), radius(radius) {}
			~LightJob() {}
			
			vec3i pos;
			vec3i radius;
		}

		Column* columns[WORLDSIZE][WORLDSIZE];
		std::list<LightJob> lightJobs;
		ColumnGenerator generator;
		DeferredContainer* renderer;
};

#endif // WORLD_HPP
