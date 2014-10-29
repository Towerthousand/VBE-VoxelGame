#ifndef DEFERREDCONTAINER_HPP
#define DEFERREDCONTAINER_HPP
#include "commons.hpp"

class DeferredContainer : public ContainerObject {
	public:

        enum DrawMode {
			Deferred = 0,
			Light,
			ShadowMap,
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

	private:
		RenderTargetBase* gBuffer;
		RenderTargetBase* sunTarget;
        mutable DrawMode drawMode;
		mutable MeshBase* quad;
};

#endif // DEFERREDCONTAINER_HPP
