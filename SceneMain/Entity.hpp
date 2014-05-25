#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "commons.hpp"
#include "Hitbox.hpp"

class Entity : public GameObject {
	public:
		Entity();
		virtual ~Entity();

		virtual void draw() const;
		virtual void update(float deltaTime);

		vec3f vel;
		vec3f acc;
		vec3f pos;
	protected:
		virtual void movePos(float deltaTime);

		Hitbox hitbox;
		vec3f scale;
};

#endif // ENTITY_HPP
