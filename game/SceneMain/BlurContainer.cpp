#include "BlurContainer.hpp"
#include "debug/Profiler.hpp"
#include "Manager.hpp"

BlurContainer::BlurContainer() : noBlurDepth(vec2ui(0), TextureFormat::DEPTH_COMPONENT32) {
	noBlurColor0.loadEmpty(vec2ui(0), TextureFormat::RGBA8);
	noBlurColor0.setFilter(GL_NEAREST, GL_NEAREST);
	noBlurColor0.setWrap(GL_CLAMP_TO_EDGE);
	noBlur = new RenderTarget(1.0f);
	noBlur->enableAttachment(RenderTargetBase::DEPTH);
	noBlur->enableAttachment(RenderTargetBase::COLOR0);
	noBlur->setBuffer(RenderTargetBase::DEPTH, &noBlurDepth);
	noBlur->setTexture(RenderTargetBase::COLOR0, &noBlurColor0);

	float blurSize = 2;
	float blurSizeDivisor = std::pow(2,blurSize);

	blurMaskColor0.loadEmpty(vec2ui(0), TextureFormat::RGBA8);
	blurMaskColor0.setFilter(GL_LINEAR, GL_LINEAR);
	blurMaskColor0.setWrap(GL_CLAMP_TO_EDGE);
	blurMask = new RenderTarget(1.0f);
	blurMask->enableAttachment(RenderTargetBase::COLOR0);
	blurMask->setTexture(RenderTargetBase::COLOR0, &blurMaskColor0);

	horitzontalBlurredColor0.loadEmpty(vec2ui(0), TextureFormat::RGBA8);
	horitzontalBlurredColor0.setFilter(GL_LINEAR, GL_LINEAR);
	horitzontalBlurredColor0.setWrap(GL_CLAMP_TO_EDGE);
	horitzontalBlurred = new RenderTarget(1.0f/blurSizeDivisor);
	horitzontalBlurred->enableAttachment(RenderTargetBase::COLOR0);
	horitzontalBlurred->setTexture(RenderTargetBase::COLOR0, &horitzontalBlurredColor0);

	blurredColor0.loadEmpty(vec2ui(0), TextureFormat::RGBA8);
	blurredColor0.setFilter(GL_LINEAR, GL_LINEAR);
	blurredColor0.setWrap(GL_CLAMP_TO_EDGE);
	blurred = new RenderTarget(1.0f/blurSizeDivisor);
	blurred->enableAttachment(RenderTargetBase::COLOR0);
	blurred->setTexture(RenderTargetBase::COLOR0, &blurredColor0);

	quad = Meshes.get("quad");
}

BlurContainer::~BlurContainer() {
	delete noBlur;
	delete blurMask;
	delete horitzontalBlurred;
	delete blurred;
}


void BlurContainer::draw() const {
	RenderTargetBase::bind(noBlur);

	ContainerObject::draw();

	float blurTime = Clock::getSeconds();

	GL_ASSERT(glBlendFunc(GL_ONE,GL_ONE));

	//BLUR MASK BUILDING
	RenderTargetBase::bind(blurMask);
	GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT));
	Programs.get("blurMaskPass")->uniform("MVP")->set(mat4f(1.0f));
	Programs.get("blurMaskPass")->uniform("color0")->set(noBlur->getTexture(RenderTargetBase::COLOR0));
	Programs.get("blurMaskPass")->uniform("invResolution")->set(vec2f(1.0f/blurMask->getWidth(), 1.0f/blurMask->getHeight()));
	quad->draw(Programs.get("blurMaskPass"));

	//BLUR
	RenderTargetBase::bind(horitzontalBlurred);
	GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT));
	if(!Keyboard::pressed(Keyboard::B)) {
		Programs.get("blurPassHoritzontal")->uniform("MVP")->set(mat4f(1.0f));
		Programs.get("blurPassHoritzontal")->uniform("RTScene")->set(blurMask->getTexture(RenderTargetBase::COLOR0));
		Programs.get("blurPassHoritzontal")->uniform("invResolution")->set(vec2f(1.0f/horitzontalBlurred->getWidth(), 1.0f/horitzontalBlurred->getHeight()));
		quad->draw(Programs.get("blurPassHoritzontal"));
	}

	RenderTargetBase::bind(blurred);
	GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT));
	if(!Keyboard::pressed(Keyboard::B)) {
		Programs.get("blurPassVertical")->uniform("MVP")->set(mat4f(1.0f));
		Programs.get("blurPassVertical")->uniform("RTBlurH")->set(horitzontalBlurred->getTexture(RenderTargetBase::COLOR0));
		Programs.get("blurPassVertical")->uniform("invResolution")->set(vec2f(1.0f/blurred->getWidth(), 1.0f/blurred->getHeight()));
		quad->draw(Programs.get("blurPassVertical"));
	}

	//BLUR + SCENE
	RenderTargetBase::bind(nullptr);
	GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT));
	Programs.get("textureToScreen")->uniform("MVP")->set(mat4f(1.0f));
	Programs.get("textureToScreen")->uniform("tex1")->set(noBlur->getTexture(RenderTargetBase::COLOR0));
	Programs.get("textureToScreen")->uniform("tex2")->set(blurred->getTexture(RenderTargetBase::COLOR0));
	Programs.get("textureToScreen")->uniform("invResolution")->set(vec2f(1.0f/(Window::getInstance()->getSize().x), 1.0f/(Window::getInstance()->getSize().y)));
	quad->draw(Programs.get("textureToScreen"));

	GL_ASSERT(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	Profiler::timeVars[Profiler::BlurPassTime] = Clock::getSeconds()-blurTime;
}
