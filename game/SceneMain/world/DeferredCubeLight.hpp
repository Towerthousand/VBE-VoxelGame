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

		void calcLight(int cx = 0, int cy = 0, int cz = 0);
		vec3f getPosition() const {return pos;}
	private:
		void update(float deltaTime);
		void draw() const;

		void count(std::pair<float, float>& pr, vec3i p, vec3i c);
		void calcQuadrant(int cx, int cy, int cz, int dx, int dy, int dz);
		float light[LIGHTSIZE*2][LIGHTSIZE*2][LIGHTSIZE*2];
		unsigned char data[LIGHTSIZE*2][LIGHTSIZE*2][LIGHTSIZE*2];
		vec3f pos;
		vec3f color;
		MeshBase* quad;
		Texture3D tex;
		DeferredContainer* renderer;
		World* world;

};

#endif // DEFERREDCUBELIGHT_HPP
