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
		const Camera* getGlobalCam() const {return cameras[NUM_SUN_CASCADES];}
		const std::vector<float>& getDepthPlanes() const {return maxZ;}
		const std::vector<mat4f>& getVPMatrices() const {return VP;}

	private:
		void recalculateCameras();
		void recalculateAABBs();
		void extend(std::vector<unsigned int> index, const AABB& occludedBox);

		float angle; //sun always moves on the x-y plane (z never changes)
		vec3f min[NUM_SUN_CASCADES+1];
		vec3f max[NUM_SUN_CASCADES+1];
		Camera* cameras[NUM_SUN_CASCADES+1];
		AABB aabbs[NUM_SUN_CASCADES+1];
		int numOccluders[NUM_SUN_CASCADES+1];
		std::vector<float> minZ;
		std::vector<float> maxZ;
		std::vector<mat4f> VP;
};

#endif // SUN_HPP
