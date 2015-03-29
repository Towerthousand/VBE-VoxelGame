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
		void makeTarget();

		RenderTarget gBuffer;

		//the textures for the gBuffer
		Texture2D GBDepth;
		Texture2D GBColor0;
		Texture2D GBColor1;

		RenderTargetLayered sunTarget;

		//the texture for the sun target
		Texture2DArray SDepth;

        mutable DrawMode drawMode;
		mutable MeshBase* quad;
};

#endif // DEFERREDCONTAINER_HPP
