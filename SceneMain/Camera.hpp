#ifndef CAMERA_HPP
#define CAMERA_HPP
#include "Frustum.hpp"

class Camera : public GameObject {
	public:
		Camera(const std::string &cameraName, const vec3f& pos = vec3f(0.0f), const vec3f& rot = vec3f(0.0f));
		virtual ~Camera();

		void update(float deltaTime);
		vec3f getWorldPos() const;
		vec3f getForward() const;

		vec3f pos;
		vec3f rot;
		mat4f projection;
		mat4f view;

		const Frustum& getFrustum() const;
	private:
		Frustum frustum;
};

#endif // CAMERA_HPP
