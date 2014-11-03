#ifndef SUN_HPP
#define SUN_HPP
#include "commons.hpp"

#define NUM_SUN_CASCADES 1

class Sun : public GameObject {
	public:
		Sun();
		~Sun();

		void update(float deltaTime);
		void updateCameras();

		vec3f getDirection() const {return vec3f(cos(angle), -sin(angle), 0.25);}
		float getAngle() const {return angle;}

		const Camera* getCam(unsigned int i) const;

	private:
		void updateSingleCam();

		float angle; //sun always moves on the x-y plane (z never changes)
		Camera* cameras[NUM_SUN_CASCADES];
		float minZ[NUM_SUN_CASCADES];
		float maxZ[NUM_SUN_CASCADES];
};

#endif // SUN_HPP
