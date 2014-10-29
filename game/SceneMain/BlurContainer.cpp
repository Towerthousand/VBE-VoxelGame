#include "BlurContainer.hpp"
#include "debug/Profiler.hpp"
#include "Manager.hpp"

BlurContainer::BlurContainer() {
	noBlur = new RenderTargetBase(1.0f);
	noBlur->addRenderBuffer(RenderTargetBase::DEPTH, TextureFormat::DEPTH_COMPONENT32);
	noBlur->addTexture(RenderTargetBase::COLOR0, TextureFormat::RGBA8);
	noBlur->getTextureForAttachment(RenderTargetBase::COLOR0)->setFilter(GL_NEAREST, GL_NEAREST);
	noBlur->getTextureForAttachment(RenderTargetBase::COLOR0)->setWrap(GL_CLAMP_TO_EDGE);

	float blurSize = 2;
	float blurSizeDivisor = std::pow(2,blurSize);

	blurMask = new RenderTargetBase(1.0f);
	blurMask->addTexture(RenderTargetBase::COLOR0, TextureFormat::RGBA8);
	blurMask->getTextureForAttachment(RenderTargetBase::COLOR0)->setFilter(GL_LINEAR, GL_LINEAR);
	blurMask->getTextureForAttachment(RenderTargetBase::COLOR0)->setWrap(GL_CLAMP_TO_EDGE);

	horitzontalBlurred = new RenderTargetBase(1.0f/blurSizeDivisor);
	horitzontalBlurred->addTexture(RenderTargetBase::COLOR0, TextureFormat::RGBA8);
	horitzontalBlurred->getTextureForAttachment(RenderTargetBase::COLOR0)->setFilter(GL_LINEAR, GL_LINEAR);
	horitzontalBlurred->getTextureForAttachment(RenderTargetBase::COLOR0)->setWrap(GL_CLAMP_TO_EDGE);

	blurred = new RenderTargetBase(1.0f/blurSizeDivisor);
	blurred->addTexture(RenderTargetBase::COLOR0, TextureFormat::RGBA8);
	blurred->getTextureForAttachment(RenderTargetBase::COLOR0)->setFilter(GL_LINEAR, GL_LINEAR);
	blurred->getTextureForAttachment(RenderTargetBase::COLOR0)->setWrap(GL_CLAMP_TO_EDGE);

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
	Programs.get("blurMaskPass")->uniform("color0")->set(noBlur->getTextureForAttachment(RenderTargetBase::COLOR0));
	Programs.get("blurMaskPass")->uniform("invResolution")->set(vec2f(1.0f/blurMask->getWidth(), 1.0f/blurMask->getHeight()));
	quad->draw(Programs.get("blurMaskPass"));

	//BLUR
	RenderTargetBase::bind(horitzontalBlurred);
	GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT));
	if(!Keyboard::pressed(Keyboard::B)) {
		Programs.get("blurPassHoritzontal")->uniform("MVP")->set(mat4f(1.0f));
		Programs.get("blurPassHoritzontal")->uniform("RTScene")->set(blurMask->getTextureForAttachment(RenderTargetBase::COLOR0));
		Programs.get("blurPassHoritzontal")->uniform("invResolution")->set(vec2f(1.0f/horitzontalBlurred->getWidth(), 1.0f/horitzontalBlurred->getHeight()));
		quad->draw(Programs.get("blurPassHoritzontal"));
	}

	RenderTargetBase::bind(blurred);
	GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT));
	if(!Keyboard::pressed(Keyboard::B)) {
		Programs.get("blurPassVertical")->uniform("MVP")->set(mat4f(1.0f));
		Programs.get("blurPassVertical")->uniform("RTBlurH")->set(horitzontalBlurred->getTextureForAttachment(RenderTargetBase::COLOR0));
		Programs.get("blurPassVertical")->uniform("invResolution")->set(vec2f(1.0f/blurred->getWidth(), 1.0f/blurred->getHeight()));
		quad->draw(Programs.get("blurPassVertical"));
	}

	//BLUR + SCENE
	RenderTargetBase::bind(nullptr);
	GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT));
	Programs.get("textureToScreen")->uniform("MVP")->set(mat4f(1.0f));
	Programs.get("textureToScreen")->uniform("tex1")->set(noBlur->getTextureForAttachment(RenderTargetBase::COLOR0));
	Programs.get("textureToScreen")->uniform("tex2")->set(blurred->getTextureForAttachment(RenderTargetBase::COLOR0));
	Programs.get("textureToScreen")->uniform("invResolution")->set(vec2f(1.0f/(Window::getInstance()->getSize().x), 1.0f/(Window::getInstance()->getSize().y)));
	quad->draw(Programs.get("textureToScreen"));

	GL_ASSERT(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	Profiler::timeVars[Profiler::BlurPassTime] = Clock::getSeconds()-blurTime;
}
