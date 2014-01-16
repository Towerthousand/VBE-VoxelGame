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

	private:
		void calcLight();
		vec3f pos;
		vec3f color;
		Model quad;
		Texture3D tex;
		DeferredContainer* renderer;
		World* world;

};

#endif // DEFERREDCUBELIGHT_HPP
