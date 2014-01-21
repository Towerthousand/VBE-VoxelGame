#include "DeferredContainer.hpp"
#include "Camera.hpp"
#include "BlurContainer.hpp"

DeferredContainer::DeferredContainer() : gBuffer(NULL), drawMode(Deferred) {
    setName("deferred");
	gBuffer = new RenderTarget();
    gBuffer->addTexture(RenderTarget::DEPTH, Texture::DEPTH_COMPONENT32); //Z-BUFFER
    gBuffer->addTexture(RenderTarget::COLOR0, Texture::RGB8); //COLOR
    gBuffer->addTexture(RenderTarget::COLOR1, Texture::RGBA16F); //NORMAL, BRIGHTNESS, SPECULAR FACTOR
	gBuffer->build();
    gBuffer->getTextureForAttachment(RenderTarget::COLOR0)->setFilter(GL_NEAREST, GL_NEAREST);
    gBuffer->getTextureForAttachment(RenderTarget::COLOR1)->setFilter(GL_NEAREST, GL_NEAREST);
	gBuffer->getTextureForAttachment(RenderTarget::DEPTH)->setFilter(GL_NEAREST, GL_NEAREST);

	sunTarget = new RenderTarget();
	sunTarget->addTexture(RenderTarget::DEPTH, Texture::DEPTH_COMPONENT32); //Z-BUFFER
	sunTarget->build();
	sunTarget->getTextureForAttachment(RenderTarget::DEPTH)->setFilter(GL_NEAREST, GL_NEAREST);
	sunTarget->getTextureForAttachment(RenderTarget::DEPTH)->setComparison(GL_LESS);

    quad.mesh = Meshes.get("quad");
    quad.program = Programs.get("ambientPass");
}

DeferredContainer::~DeferredContainer() {
	delete gBuffer;
}

void DeferredContainer::update(float deltaTime) {
    ContainerObject::update(deltaTime);
}

void DeferredContainer::draw() const {
	//"The Screen". It may not be actually the screen since a upper container might be postprocessing
	RenderTarget* screen = RenderTarget::getCurrent();
	//G BUFFER
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND); //no transparency whatsoever

	drawMode = Deferred;
	RenderTarget::bind(gBuffer);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	ContainerObject::draw();

	RenderTarget::bind(sunTarget);
	glClear(GL_DEPTH_BUFFER_BIT);
	ContainerObject::draw();

	RenderTarget::bind(screen);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	//DEFERRED LIGHTS
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE); //additive
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_ALWAYS);
	drawMode = Light;
	ContainerObject::draw();

	//AMBIENT LIGHT
	quad.program = Programs.get("ambientPass");
	quad.program->uniform("MVP")->set(mat4f(1.0f));
	quad.program->uniform("color0")->set(getColor0());
	quad.program->uniform("color1")->set(getColor1());
	quad.program->uniform("depth")->set(getDepth());
	quad.program->uniform("invResolution")->set(vec2f(1.0f/SCRWIDTH, 1.0f/SCRHEIGHT));
	quad.draw();

	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //forward rendering blending
}

DeferredContainer::DrawMode DeferredContainer::getMode() const {
    return drawMode;
}

Texture2D *DeferredContainer::getColor0() const {
    return gBuffer->getTextureForAttachment(RenderTarget::COLOR0);
}

Texture2D *DeferredContainer::getColor1() const {
    return gBuffer->getTextureForAttachment(RenderTarget::COLOR1);
}

Texture2D* DeferredContainer::getDepth() const {
    return gBuffer->getTextureForAttachment(RenderTarget::DEPTH);
}

