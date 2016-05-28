#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "commons.hpp"
#include "Hitbox.hpp"

class Entity : public GameObject {
    public:
        Entity();
        virtual ~Entity();

        vec3f getPosition() const { return pos;}

    protected:
        virtual void draw() const;
        virtual void update(float deltaTime);
        virtual void movePos(float deltaTime);

        vec3f vel = vec3f(0.0f);
        vec3f acc = vec3f(0.0f);
        Hitbox* hitbox = nullptr;
        vec3f scale = vec3f(1.0f);
        vec3f pos = vec3f(0.0f);
};

#endif // ENTITY_HPP
