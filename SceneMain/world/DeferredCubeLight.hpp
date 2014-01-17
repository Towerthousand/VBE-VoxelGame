#ifndef DEFERREDCUBELIGHT_HPP
#define DEFERREDCUBELIGHT_HPP
#include "commons.hpp"

#define LIGHTSIZE 16

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
		void count(float light[LIGHTSIZE*2][LIGHTSIZE*2][LIGHTSIZE*2], std::pair<float, float>& p, float px, float py, float pz, int x, int y, int z);
		float light[LIGHTSIZE*2][LIGHTSIZE*2][LIGHTSIZE*2];
		unsigned char data[LIGHTSIZE*2][LIGHTSIZE*2][LIGHTSIZE*2];
		vec3f pos;
		vec3f color;
		Model quad;
		Texture3D tex;
		DeferredContainer* renderer;
		World* world;

};

#endif // DEFERREDCUBELIGHT_HPP
