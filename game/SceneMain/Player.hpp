#ifndef PLAYER_HPP
#define PLAYER_HPP
#include "Entity.hpp"

class Camera;
class Player : public Entity {
    public:
        Player();
        ~Player();

        void update(float deltaTime);
        void fixedUpdate(float deltaTime);
        Camera* getCam() const { return cam; }
        vec2f getCamFov() const { return fov;}

    private:
        void processKeys(float deltaTime);
        void traceView();

        Camera* cam = nullptr;
        unsigned int selectedID = 1; //current blockID, used to place blocks
        vec3i targetedBlock = vec3i(0);
        vec3i targetedBlockEnter = vec3i(0);
        float xRot = 0.0f;
        bool onFloor = true;
        bool isJumping = false;
        bool targetsBlock = false;
        vec3f lastPos = vec3f(0.0f);
        vec2f fov;
};

#endif // PLAYER_HPP
