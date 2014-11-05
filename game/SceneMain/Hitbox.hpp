#ifndef HITBOX_HPP
#define HITBOX_HPP
#include "commons.hpp"

class Hitbox : public GameObject {
	public:
		enum hitboxType {
			BOX = 0,
			POINT
		};

		Hitbox(hitboxType type, const vec3f &radius = vec3f(0,0,0));
		~Hitbox();

		bool collidesWithWorld( const vec3f &offset = vec3f(0,0,0)) const;
		bool collidesWithHitbox(const Hitbox& hitbox, const vec3f &offset = vec3f(0,0,0)) const;

		vec3f getWorldPos() const;

		vec3f radius;
		hitboxType type;
	private:
		bool pointCollidesWithWorld(const vec3f& point) const;
};

#endif // HITBOX_HPP
