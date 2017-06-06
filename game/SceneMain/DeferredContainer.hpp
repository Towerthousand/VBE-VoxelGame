#ifndef DEFERREDCONTAINER_HPP
#define DEFERREDCONTAINER_HPP
#include "commons.hpp"

class DeferredContainer : public ContainerObject {
    public:

        enum DrawMode {
            Deferred = 0,
            Light,
            ShadowMap,
            TransShadowMap,
            Forward
        };

        DeferredContainer();
        ~DeferredContainer();

        void update(float deltaTime);
        void draw() const;
        DrawMode getMode() const;
        Texture2D* getColor0() const;
        Texture2D* getColor1() const;
        Texture2D* getDepth() const;
        Texture2DArray* getTransSunDepth() const;
        Texture2DArray* getSunDepth() const;

    private:
        void makeTarget();

        RenderTarget gBuffer;

        //the textures for the gBuffer
        Texture2D GBDepth;
        Texture2D GBColor0;
        Texture2D GBColor1;

        RenderTargetLayered sunTarget;
        RenderTargetLayered sunTargetTrans;

        //the texture for the sun target
        Texture2DArray SDepth;
        Texture2DArray SDepthTrans;

        mutable DrawMode drawMode = Deferred;
        mutable MeshBase* quad = nullptr;
};

#endif // DEFERREDCONTAINER_HPP
