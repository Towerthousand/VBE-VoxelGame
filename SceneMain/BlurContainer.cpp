#include "BlurContainer.hpp"
#include "debug/Profiler.hpp"

BlurContainer::BlurContainer() {
	noBlur = new RenderTarget();
	noBlur->addRenderBuffer(RenderTarget::DEPTH, Texture::DEPTH_COMPONENT32);
	noBlur->addTexture(RenderTarget::COLOR0, Texture::RGBA8);
	noBlur->getTextureForAttachment(RenderTarget::COLOR0)->setFilter(GL_NEAREST, GL_NEAREST);
	noBlur->getTextureForAttachment(RenderTarget::COLOR0)->setWrap(GL_CLAMP_TO_EDGE);

	float blurSize = 2;
	float blurSizeDivisor = std::pow(2,blurSize);

	blurMask = new RenderTarget();
	blurMask->addTexture(RenderTarget::COLOR0, Texture::RGBA8);
	blurMask->getTextureForAttachment(RenderTarget::COLOR0)->setFilter(GL_LINEAR, GL_LINEAR);
	blurMask->getTextureForAttachment(RenderTarget::COLOR0)->setWrap(GL_CLAMP_TO_EDGE);

	horitzontalBlurred = new RenderTarget(1.0f/blurSizeDivisor);
	horitzontalBlurred->addTexture(RenderTarget::COLOR0, Texture::RGBA8);
	horitzontalBlurred->getTextureForAttachment(RenderTarget::COLOR0)->setFilter(GL_LINEAR, GL_LINEAR);
	horitzontalBlurred->getTextureForAttachment(RenderTarget::COLOR0)->setWrap(GL_CLAMP_TO_EDGE);

	blurred = new RenderTarget(1.0f/blurSizeDivisor);
	blurred->addTexture(RenderTarget::COLOR0, Texture::RGBA8);
	blurred->getTextureForAttachment(RenderTarget::COLOR0)->setFilter(GL_LINEAR, GL_LINEAR);
	blurred->getTextureForAttachment(RenderTarget::COLOR0)->setWrap(GL_CLAMP_TO_EDGE);

	quad.mesh = Meshes.get("quad");
	quad.program = Programs.get("ambientPass");
}

BlurContainer::~BlurContainer() {
	delete noBlur;
	delete blurMask;
	delete horitzontalBlurred;
	delete blurred;
}


void BlurContainer::draw() const {
	RenderTarget::bind(noBlur);

	ContainerObject::draw();

	float blurTime = Environment::getClock();

	GL_ASSERT(glBlendFunc(GL_ONE,GL_ONE));

	//BLUR MASK BUILDING
	RenderTarget::bind(blurMask);
	GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT));
	quad.program = Programs.get("blurMaskPass");
	quad.program->uniform("MVP")->set(mat4f(1.0f));
	quad.program->uniform("color0")->set(noBlur->getTextureForAttachment(RenderTarget::COLOR0));
	quad.program->uniform("invResolution")->set(vec2f(1.0f/blurMask->getWidth(), 1.0f/blurMask->getHeight()));
	quad.draw();

	//BLUR
	RenderTarget::bind(horitzontalBlurred);
	GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT));
	if(!Environment::getKeyboard()->isKeyHeld(Keyboard::B)) {
		quad.program = Programs.get("blurPassHoritzontal");
		quad.program->uniform("MVP")->set(mat4f(1.0f));
		quad.program->uniform("RTScene")->set(blurMask->getTextureForAttachment(RenderTarget::COLOR0));
		quad.program->uniform("invResolution")->set(vec2f(1.0f/horitzontalBlurred->getWidth(), 1.0f/horitzontalBlurred->getHeight()));
		quad.draw();
	}

	RenderTarget::bind(blurred);
	GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT));
	if(!Environment::getKeyboard()->isKeyHeld(Keyboard::B)) {
		quad.program = Programs.get("blurPassVertical");
		quad.program->uniform("MVP")->set(mat4f(1.0f));
		quad.program->uniform("RTBlurH")->set(horitzontalBlurred->getTextureForAttachment(RenderTarget::COLOR0));
		quad.program->uniform("invResolution")->set(vec2f(1.0f/blurred->getWidth(), 1.0f/blurred->getHeight()));
		quad.draw();
	}

	//BLUR + SCENE
	RenderTarget::bind(nullptr);
	GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT));
	quad.program = Programs.get("textureToScreen");
	quad.program->uniform("MVP")->set(mat4f(1.0f));
    quad.program->uniform("tex1")->set(noBlur->getTextureForAttachment(RenderTarget::COLOR0));
	quad.program->uniform("tex2")->set(blurred->getTextureForAttachment(RenderTarget::COLOR0));
	quad.program->uniform("invResolution")->set(vec2f(1.0f/(Environment::getScreen()->getWidth()), 1.0f/(Environment::getScreen()->getHeight())));
	quad.draw();

	GL_ASSERT(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	Profiler::timeVars[Profiler::BlurPassTime] = Environment::getClock()-blurTime;
}
