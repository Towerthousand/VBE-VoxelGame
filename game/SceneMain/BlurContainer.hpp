#ifndef BLURCONTAINER_HPP
#define BLURCONTAINER_HPP
#include "commons.hpp"

class BlurContainer : public ContainerObject {
	public:
		BlurContainer();
		~BlurContainer();

		virtual void draw() const;
	private:

		RenderTarget* noBlur;
		RenderBuffer noBlurDepth;
		Texture2D noBlurColor0;

		RenderTarget* blurMask;
		Texture2D blurMaskColor0;

		RenderTarget* horitzontalBlurred;
		Texture2D horitzontalBlurredColor0;

		RenderTarget* blurred;
		Texture2D blurredColor0;

		mutable MeshBase* quad;
};

#endif // BLURCONTAINER_HPP
