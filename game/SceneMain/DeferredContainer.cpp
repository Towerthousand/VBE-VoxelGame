#include "DeferredContainer.hpp"
#include "BlurContainer.hpp"
#include "world/World.hpp"
#include "world/Chunk.hpp"
#include "world/Column.hpp"
#include "world/Sun.hpp"
#include "debug/Profiler.hpp"
#include "Manager.hpp"

DeferredContainer::DeferredContainer() : gBuffer(NULL), drawMode(Deferred) {
	setName("deferred");
	gBuffer = new RenderTargetBase(1.0f);
	gBuffer->addTexture(RenderTargetBase::DEPTH, TextureFormat::DEPTH_COMPONENT32); //Z-BUFFER
	gBuffer->addTexture(RenderTargetBase::COLOR0, TextureFormat::RGB8); //COLOR
	gBuffer->addTexture(RenderTargetBase::COLOR1, TextureFormat::RGBA16F); //NORMAL, BRIGHTNESS, SPECULAR FACTOR
	gBuffer->getTextureForAttachment(RenderTargetBase::COLOR0)->setFilter(GL_NEAREST, GL_NEAREST);
	gBuffer->getTextureForAttachment(RenderTargetBase::COLOR1)->setFilter(GL_NEAREST, GL_NEAREST);
	gBuffer->getTextureForAttachment(RenderTargetBase::DEPTH)->setFilter(GL_NEAREST, GL_NEAREST);

	sunTarget = new RenderTargetBase(2048,2048);
	sunTarget->addTexture(RenderTargetBase::DEPTH, TextureFormat::DEPTH_COMPONENT32F); //Z-BUFFER
	sunTarget->getTextureForAttachment(RenderTargetBase::DEPTH)->setFilter(GL_NEAREST, GL_NEAREST);
	quad = Meshes.get("quad");
}

DeferredContainer::~DeferredContainer() {
	delete gBuffer;
}

void DeferredContainer::update(float deltaTime) {
	ContainerObject::update(deltaTime);
}

void DeferredContainer::draw() const {
	//"The Screen". It may not be actually the screen since a upper container might be postprocessing
	const RenderTargetBase* screen = RenderTargetBase::getCurrent();

	GL_ASSERT(glEnable(GL_DEPTH_TEST));
	GL_ASSERT(glDisable(GL_BLEND));

	//Deferred pass
	float deferredTime = Clock::getSeconds();
	drawMode = Deferred;
	RenderTargetBase::bind(gBuffer);
	GL_ASSERT(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
	ContainerObject::draw();
	Profiler::timeVars[Profiler::DeferredPassTime] = Clock::getSeconds()-deferredTime;

	//Shadowmap pass
	float shadowTime = Clock::getSeconds();
	drawMode = ShadowMap;
	RenderTargetBase::bind(sunTarget);
	GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	ContainerObject::draw();
	Profiler::timeVars[Profiler::ShadowBuildPassTime] = Clock::getSeconds()-shadowTime;

	//bind output texture (screen)
	RenderTargetBase::bind(screen);
	GL_ASSERT(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

	//Light pass
	float lightTime = Clock::getSeconds();
	GL_ASSERT(glEnable(GL_BLEND));
	GL_ASSERT(glBlendFunc(GL_ONE, GL_ONE)); //additive
	GL_ASSERT(glDepthMask(GL_TRUE));
	GL_ASSERT(glDepthFunc(GL_ALWAYS));
	drawMode = Light;
	ContainerObject::draw();
	Profiler::timeVars[Profiler::LightPassTime] = Clock::getSeconds()-lightTime;

	//Ambient+Visibility pass
	float ambinentShadowPass = Clock::getSeconds();
	Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");
	Camera* sCam = (Camera*)getGame()->getObjectByName("sunCamera");
	Programs.get("ambientPass")->uniform("MVP")->set(mat4f(1.0f));
	Programs.get("ambientPass")->uniform("camMV")->set(cam->getView()*fullTransform);
	Programs.get("ambientPass")->uniform("color0")->set(getColor0());
	Programs.get("ambientPass")->uniform("color1")->set(getColor1());
	Programs.get("ambientPass")->uniform("invResolution")->set(vec2f(1.0f/screen->getWidth(), 1.0f/screen->getHeight()));
	Programs.get("ambientPass")->uniform("invCamProj")->set(glm::inverse(cam->projection));
	Programs.get("ambientPass")->uniform("invCamView")->set(glm::inverse(cam->getView()));
	Programs.get("ambientPass")->uniform("lightDir")->set(sCam->getForward());
	glm::mat4 biasMatrix( //gets coords from [-1..1] to [0..1]
						  0.5, 0.0, 0.0, 0.0,
						  0.0, 0.5, 0.0, 0.0,
						  0.0, 0.0, 0.5, 0.0,
						  0.5, 0.5, 0.5, 1.0
						  );
	Programs.get("ambientPass")->uniform("depthMVP")->set(biasMatrix*(sCam->projection*sCam->getView()*fullTransform));
	Programs.get("ambientPass")->uniform("depth")->set(gBuffer->getTextureForAttachment(RenderTargetBase::DEPTH));
	Programs.get("ambientPass")->uniform("sunDepth")->set(sunTarget->getTextureForAttachment(RenderTargetBase::DEPTH));
	quad->draw(Programs.get("ambientPass"));
	Profiler::timeVars[Profiler::AmbinentShadowPassTime] = Clock::getSeconds()-ambinentShadowPass;

	//Forward pass
	float forwardPass = Clock::getSeconds();
	GL_ASSERT(glDepthFunc(GL_LEQUAL));
	GL_ASSERT(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); //forward rendering blending
	drawMode = Forward;
	ContainerObject::draw();
	Profiler::timeVars[Profiler::ForwardPassTime] = Clock::getSeconds()-forwardPass;
}

DeferredContainer::DrawMode DeferredContainer::getMode() const {
	return drawMode;
}

Texture2D *DeferredContainer::getColor0() const {
	return gBuffer->getTextureForAttachment(RenderTargetBase::COLOR0);
}

Texture2D *DeferredContainer::getColor1() const {
	return gBuffer->getTextureForAttachment(RenderTargetBase::COLOR1);
}

Texture2D* DeferredContainer::getDepth() const {
	return gBuffer->getTextureForAttachment(RenderTargetBase::DEPTH);
}

