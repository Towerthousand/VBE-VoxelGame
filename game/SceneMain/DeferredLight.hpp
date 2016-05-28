#ifndef DEFERREDLIGHT_HPP
#define DEFERREDLIGHT_HPP
#include "commons.hpp"

class DeferredContainer;
class DeferredLight : public GameObject{
    public:
        DeferredLight();
        virtual ~DeferredLight();

        virtual void update(float deltaTime);
        void draw() const;
        vec3f pos = vec3f(0.0f);
        vec3f color = vec3f(1.0f);
        float radius = 30.0f;
    private:
        DeferredContainer* renderer = nullptr;
        MeshBase* quad = nullptr;
};

#endif // DEFERREDLIGHT_HPP
