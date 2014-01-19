#ifndef DEFERREDLIGHT_HPP
#define DEFERREDLIGHT_HPP
#include "commons.hpp"

class DeferredContainer;
class DeferredLight : public GameObject{
	public:
		DeferredLight();
		virtual ~DeferredLight();

		virtual void update(float deltaTime);
		void draw() const;
        vec3f pos;
		vec3f color;
        float radius;
	private:
		DeferredContainer* renderer;
        Model quad;
};

#endif // DEFERREDLIGHT_HPP
