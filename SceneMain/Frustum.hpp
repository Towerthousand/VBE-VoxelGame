#ifndef FRUSTUM_HPP
#define FRUSTUM_HPP
#include "commons.hpp"

class Frustum {
		class Plane {
			public:
				Plane(vec3f p0, vec3f p1, vec3f p2);
				Plane(vec3f n, vec3f p);
				Plane(vec3f n, float d);
				Plane();
				~Plane();

				bool sphereInside(vec3f p, float r) const;
				bool pointInside(vec3f p) const;

				vec3f n;
				float d;
		};

	public:
		enum { //frustrum planes
			TOP = 0,
			BOTTOM,
			LEFT,
			RIGHT,
			NEAR,
			FAR
		};

		Frustum();
		~Frustum();

		void calculate(mat4f viewMatrix);
		bool insideFrustum(const vec3f& center, float radius) const;

	private:
		Plane planes[4];
};

#endif // FRUSTUM_HPP
