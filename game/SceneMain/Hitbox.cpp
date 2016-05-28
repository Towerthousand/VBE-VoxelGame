#include "Hitbox.hpp"
#include "world/World.hpp"

Hitbox::Hitbox(hitboxType type, const vec3f &radius) : radius(radius), type(type) {
}

Hitbox::~Hitbox() {
}

bool Hitbox::collidesWithWorld(const vec3f &offset) const {
    switch (type) {
        case (BOX): {
            vec3f newPos = getWorldPos()+offset;
            for(float x = -radius.x; x <= radius.x+0.3; x += 0.3)
                for(float y = -radius.y; y <= radius.y+0.3; y += 0.3)
                    for(float z = -radius.z; z <= radius.z+0.3; z += 0.3) {
                        vec3f point(std::fmin(x, radius.x), std::fmin(y, radius.y),std::fmin(z, radius.z));
                        if(pointCollidesWithWorld(point+newPos)) {
                            return true;
                        }
                    }
            break;
        }
        case (POINT):
            return pointCollidesWithWorld(getWorldPos()+offset);
            break;
        default:
            break;
    }
    return false;
}

bool Hitbox::collidesWithHitbox(const Hitbox &hitbox, const vec3f &offset) const {
    return false; //TODO
}

vec3f Hitbox::getWorldPos() const {
    return vec3f(fullTransform*vec4f(0,0,0,1));
}

bool Hitbox::pointCollidesWithWorld(const vec3f& point) const {
    World* world = (World*)getGame()->getObjectByName("world");
    if(world->getCube(floor(point.x),floor(point.y),floor(point.z)) != 0)
        return true;
    return false;
}
