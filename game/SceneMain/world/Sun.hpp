#ifndef SUN_HPP
#define SUN_HPP
#include "commons.hpp"

#define NUM_SUN_CASCADES 4

class Sun : public GameObject {
	public:
		Sun();
		~Sun();

		void update(float deltaTime);
		void updateCameras();

		vec3f getDirection() const {return vec3f(cos(angle), -sin(angle), 0.25);}
		float getAngle() const {return angle;}

		const Camera* getCam(unsigned int i) const {return cameras[i];}
		const Camera* getGlobalCam() const {return globalCam;}
		const std::vector<float>& getDepthPlanes() const {return maxZ;}
		const std::vector<mat4f>& getVPMatrices() const {return VP;}

	private:
		void calculateAABB(unsigned int camID);
		void updateSingleCam(Camera* cam, const AABB& occludedBox);

		float angle; //sun always moves on the x-y plane (z never changes)
		Camera* cameras[NUM_SUN_CASCADES];
		Camera* globalCam;
		float minZ[NUM_SUN_CASCADES];
		std::vector<float> maxZ;
		AABB aabbs[NUM_SUN_CASCADES];
		int numOccluders[NUM_SUN_CASCADES];
		std::vector<mat4f> VP;
};

#endif // SUN_HPP
