#include "Entity.hpp"

Entity::Entity() : hitbox(new Hitbox(Hitbox::POINT)) {
    hitbox->addTo(this);
}

Entity::~Entity() {
}

void Entity::update(float deltaTime) {
    (void) deltaTime;
}

void Entity::draw() const {
}

void Entity::movePos(float deltaTime) { //collisons
    vel += acc*deltaTime; //a = const, v = at
    vec3f disp = vel*deltaTime; //deltaX = vt
    if(hitbox->collidesWithWorld(vec3f(disp.x,0,0))) {
        if ((!hitbox->collidesWithWorld(vec3f(disp.x,1.01,0)) || !hitbox->collidesWithWorld(vec3f(disp.x,0.5,0))) &&
            hitbox->collidesWithWorld(vec3f(0,-2,0))) {
            //this is for auto climbing. First check: So that it can go through underground
            //staircases. Second check: So it won't climb if it's in the air.
            pos.y += 15*deltaTime;
            vel.y += 2*deltaTime;
        }
        float min = 0;
        float max = 1;
        while(max-min > 0.001) { //search for the maximum distance you can move
            float m = (max+min)/2;
            if(hitbox->collidesWithWorld(vec3f(disp.x*m,0,0)))
                max = m;
            else
                min = m;
        }
        vel.x = 0;
        disp.x *= min;
    }

    if(hitbox->collidesWithWorld(vec3f(0,0,disp.z))) {
        if ((!hitbox->collidesWithWorld(vec3f(0,1.01,disp.z)) || !hitbox->collidesWithWorld(vec3f(0,0.5,disp.z))) &&
            hitbox->collidesWithWorld(vec3f(0,-2,0))) {
            //this is for auto climbing. First check: So that it can go through underground
            //staircases. Second check: So it won't climb if it's in the air.
            pos.y += 15*deltaTime;;
            vel.y += 2*deltaTime;;
        }
        float min = 0;
        float max = 1;
        while(max-min > 0.001) { //search for the maximum distance you can move
            float m = (max+min)/2;
            if(hitbox->collidesWithWorld(vec3f(0,0,disp.z*m)))
                max = m;
            else
                min = m;
        }
        vel.z = 0;
        disp.z *= min;
    }

    disp.y = vel.y * deltaTime; //corrected displacement for the climbing feature

    if(hitbox->collidesWithWorld(vec3f(0,disp.y,0))) {
        float min = 0;
        float max = 1;
        while(max-min > 0.001) { //search for the maximum distance you can move
            float m = (max+min)/2;
            if(hitbox->collidesWithWorld(vec3f(0,disp.y*m,0)))
                max = m;
            else
                min = m;
        }
        vel.y = 0;
        disp.y *= min;
    }

    pos += disp;
}
