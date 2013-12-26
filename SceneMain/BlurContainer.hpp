#ifndef BLURCONTAINER_HPP
#define BLURCONTAINER_HPP
#include "commons.hpp"

class BlurContainer : public ContainerObject {
	public:
		BlurContainer();
		~BlurContainer();

		virtual void draw() const;

		RenderTarget* noBlur;
		RenderTarget* blurMask;
		RenderTarget* horitzontalBlurred;
		RenderTarget* blurred;
		mutable Model quad;
	private:
};

#endif // BLURCONTAINER_HPP
