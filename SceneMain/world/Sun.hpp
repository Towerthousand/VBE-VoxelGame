#ifndef SUN_HPP
#define SUN_HPP
#include "commons.hpp"

class Sun : public GameObject {
	public:
		Sun();
		~Sun();

		void update(float deltaTime);

		vec3f getDirection() const {return vec3f(cos(angle), -sin(angle), 0);}
		float getAngle() const {return angle;}

	private:
		float angle; //sun always moves on the x-y plane (z never changes)
		Camera* cam;
};

#endif // SUN_HPP
