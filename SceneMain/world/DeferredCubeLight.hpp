#ifndef DEFERREDCUBELIGHT_HPP
#define DEFERREDCUBELIGHT_HPP
#include "commons.hpp"

class World;
class DeferredContainer;
class DeferredCubeLight : public GameObject {
	public:
		DeferredCubeLight(const vec3f& pos, const vec3f& color);
		~DeferredCubeLight();

		void update(float deltaTime);
		void draw() const;

		void calcLight();
	private:
		void count(float light[32][32][32], std::pair<double, double>& p, double px, double py, double pz, int x, int y, int z);
		vec3f pos;
		vec3f color;
		Model quad;
		Texture3D tex;
		DeferredContainer* renderer;
		World* world;

};

#endif // DEFERREDCUBELIGHT_HPP
