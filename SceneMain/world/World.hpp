#ifndef WORLD_HPP
#define WORLD_HPP
#include "commons.hpp"
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
		void drawShadowMaps() const;

		bool outOfBounds(int x, int y, int z) const;
		unsigned int getCube(int x, int y, int z) const;
		Column* getColumn(int x, int y, int z) const;
		Camera* getCamera() const;

		void setCube(int x, int y, int z, unsigned int cube);
	private:
		Column* columns[WORLDSIZE][WORLDSIZE];
		ColumnGenerator generator;
		DeferredContainer* renderer;
};

#endif // WORLD_HPP
