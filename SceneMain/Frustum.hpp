#ifndef FRUSTUM_HPP
#define FRUSTUM_HPP
#include "commons.hpp"

class Frustum {
		enum { //frustrum planes
			TOP = 0,
			BOTTOM,
			LEFT,
			RIGHT,
			NEAR,
			FAR
		};

		class Plane {
			public:
				Plane(vec3f p0, vec3f p1, vec3f p2); //Three points
				Plane(vec3f n, vec3f p); //point and normal
				Plane(vec3f n, float d); //Equation Ax +By +Cz + D = 0, where n = (A,B,C)
				Plane(vec4f ABCD); //Equation Ax + By + Cz + D = 0, where ABCD = (A,B,C,D)
				Plane(); //generated plane will be invalid by default
				~Plane();

				bool sphereInside(vec3f p, float r) const;
				bool pointInside(vec3f p) const;

			private:
				vec3f n;
				float d;
		};

	public:

		Frustum();
		~Frustum();

		void calculate(mat4f VP);
		bool insideFrustum(const vec3f& center, float radius) const;

	private:
		Plane planes[4];
};

#endif // FRUSTUM_HPP
