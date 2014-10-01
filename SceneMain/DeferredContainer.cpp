#include "DeferredContainer.hpp"
#include "BlurContainer.hpp"
#include "world/World.hpp"
#include "world/Chunk.hpp"
#include "world/Column.hpp"
#include "world/Sun.hpp"
#include "debug/Profiler.hpp"

DeferredContainer::DeferredContainer() : gBuffer(NULL), drawMode(Deferred) {
	setName("deferred");
	gBuffer = new RenderTarget(1.0f);
	gBuffer->addTexture(RenderTarget::DEPTH, Texture::DEPTH_COMPONENT32); //Z-BUFFER
	gBuffer->addTexture(RenderTarget::COLOR0, Texture::RGB8); //COLOR
	gBuffer->addTexture(RenderTarget::COLOR1, Texture::RGBA16F); //NORMAL, BRIGHTNESS, SPECULAR FACTOR
	gBuffer->getTextureForAttachment(RenderTarget::COLOR0)->setFilter(GL_NEAREST, GL_NEAREST);
	gBuffer->getTextureForAttachment(RenderTarget::COLOR1)->setFilter(GL_NEAREST, GL_NEAREST);
	gBuffer->getTextureForAttachment(RenderTarget::DEPTH)->setFilter(GL_NEAREST, GL_NEAREST);

	sunTarget = new RenderTarget(3.0f);
	sunTarget->addTexture(RenderTarget::DEPTH, Texture::DEPTH_COMPONENT32); //Z-BUFFER
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
	const RenderTarget* screen = RenderTarget::getCurrent();

	GL_ASSERT(glEnable(GL_DEPTH_TEST));
	GL_ASSERT(glDisable(GL_BLEND));

	//Deferred pass
	float deferredTime = Environment::getClock();
	drawMode = Deferred;
	RenderTarget::bind(gBuffer);
	GL_ASSERT(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
	ContainerObject::draw();
	Profiler::timeVars[Profiler::DeferredPassTime] = Environment::getClock()-deferredTime;

	//Shadowmap pass
	float shadowTime = Environment::getClock();
	drawMode = ShadowMap;
	RenderTarget::bind(sunTarget);
	GL_ASSERT(glClear(GL_DEPTH_BUFFER_BIT));
	ContainerObject::draw();
	Profiler::timeVars[Profiler::ShadowBuildPassTime] = Environment::getClock()-shadowTime;

	//bind output texture (screen)
	RenderTarget::bind(screen);
	GL_ASSERT(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

	//Light pass
	float lightTime = Environment::getClock();
	GL_ASSERT(glEnable(GL_BLEND));
	GL_ASSERT(glBlendFunc(GL_ONE, GL_ONE)); //additive
	GL_ASSERT(glDepthMask(GL_TRUE));
	GL_ASSERT(glDepthFunc(GL_ALWAYS));
	drawMode = Light;
	ContainerObject::draw();
	Profiler::timeVars[Profiler::LightPassTime] = Environment::getClock()-lightTime;

	//Ambient+Visibility pass
	float ambinentShadowPass = Environment::getClock();
	Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");
	Camera* sCam = (Camera*)getGame()->getObjectByName("sunCamera");
	quad.program = Programs.get("ambientPass");
	quad.program->uniform("MVP")->set(mat4f(1.0f));
	quad.program->uniform("camMV")->set(cam->getView()*fullTransform);
	quad.program->uniform("color0")->set(getColor0());
	quad.program->uniform("color1")->set(getColor1());
	quad.program->uniform("invResolution")->set(vec2f(1.0f/screen->getWidth(), 1.0f/screen->getHeight()));
	quad.program->uniform("invCamProj")->set(glm::inverse(cam->projection));
	quad.program->uniform("invCamView")->set(glm::inverse(cam->getView()));
	quad.program->uniform("lightDir")->set(sCam->getForward());
	glm::mat4 biasMatrix( //gets coords from [-1..1] to [0..1]
						  0.5, 0.0, 0.0, 0.0,
						  0.0, 0.5, 0.0, 0.0,
						  0.0, 0.0, 0.5, 0.0,
						  0.5, 0.5, 0.5, 1.0
						  );
	quad.program->uniform("depthMVP")->set(biasMatrix*(sCam->projection*sCam->getView()*fullTransform));
	quad.program->uniform("depth")->set(gBuffer->getTextureForAttachment(RenderTarget::DEPTH));
	quad.program->uniform("sunDepth")->set(sunTarget->getTextureForAttachment(RenderTarget::DEPTH));
	quad.draw();
	Profiler::timeVars[Profiler::AmbinentShadowPassTime] = Environment::getClock()-ambinentShadowPass;

	//Forward pass
	float forwardPass = Environment::getClock();
	GL_ASSERT(glDepthFunc(GL_LEQUAL));
	GL_ASSERT(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); //forward rendering blending
	drawMode = Forward;
	ContainerObject::draw();
	Profiler::timeVars[Profiler::ForwardPassTime] = Environment::getClock()-forwardPass;
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

