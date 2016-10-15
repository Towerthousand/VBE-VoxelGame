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
    disp += acc*deltaTime; //a = const, dist = vt + at2
    vec3f dist = disp;
    if(hitbox->collidesWithWorld(vec3f(dist.x,0,0))) {
        if ((!hitbox->collidesWithWorld(vec3f(dist.x,1.01,0)) || !hitbox->collidesWithWorld(vec3f(dist.x,0.5,0))) &&
            hitbox->collidesWithWorld(vec3f(0,-2,0))) {
            //this is for auto climbing.
            //First check: So that it can go through underground staircases.
            //Second check: So it won't climb if it's in the air.
            pos.y += 15*deltaTime;
            disp.y += 2*deltaTime;
        }
        float min = 0;
        float max = 1;
        while(max-min > 0.001) { //search for the maximum distance you can move
            float m = (max+min)/2;
            if(hitbox->collidesWithWorld(vec3f(dist.x*m,0,0)))
                max = m;
            else
                min = m;
        }
        disp.x = 0;
        dist.x *= min;
    }

    if(hitbox->collidesWithWorld(vec3f(0,0,dist.z))) {
        if ((!hitbox->collidesWithWorld(vec3f(0,1.01,dist.z)) || !hitbox->collidesWithWorld(vec3f(0,0.5,dist.z))) &&
            hitbox->collidesWithWorld(vec3f(0,-2,0))) {
            //this is for auto climbing.
            //First check: So that it can go through underground staircases.
            //Second check: So it won't climb if it's in the air.
            pos.y += 15*deltaTime;;
            disp.y += 2*deltaTime;;
        }
        float min = 0;
        float max = 1;
        while(max-min > 0.001) { //search for the maximum distance you can move
            float m = (max+min)/2;
            if(hitbox->collidesWithWorld(vec3f(0,0,dist.z*m)))
                max = m;
            else
                min = m;
        }
        disp.z = 0;
        dist.z *= min;
    }

    dist.y = disp.y * deltaTime; //corrected displacement for the climbing feature

    if(hitbox->collidesWithWorld(vec3f(0,dist.y,0))) {
        float min = 0;
        float max = 1;
        while(max-min > 0.001) { //search for the maximum distance you can move
            float m = (max+min)/2;
            if(hitbox->collidesWithWorld(vec3f(0,dist.y*m,0)))
                max = m;
            else
                min = m;
        }
        disp.y = 0;
        dist.y *= min;
    }

    pos += dist;
}
