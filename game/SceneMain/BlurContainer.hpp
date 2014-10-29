#ifndef BLURCONTAINER_HPP
#define BLURCONTAINER_HPP
#include "commons.hpp"

class BlurContainer : public ContainerObject {
	public:
		BlurContainer();
		~BlurContainer();

		virtual void draw() const;

		RenderTargetBase* noBlur;
		RenderTargetBase* blurMask;
		RenderTargetBase* horitzontalBlurred;
		RenderTargetBase* blurred;
		mutable MeshBase* quad;
	private:
};

#endif // BLURCONTAINER_HPP
