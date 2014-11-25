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
	//init g buffer texture
	GBDepth.loadEmpty(vec2ui(1), TextureFormat::DEPTH_COMPONENT32F);
	GBDepth.setFilter(GL_NEAREST, GL_NEAREST);
	GBColor0.loadEmpty(vec2ui(1), TextureFormat::RGBA16F);
	GBColor0.setFilter(GL_NEAREST, GL_NEAREST);
	GBColor1.loadEmpty(vec2ui(1), TextureFormat::RGBA16F);
	GBColor1.setFilter(GL_NEAREST, GL_NEAREST);
	gBuffer = new RenderTarget(1.0f);
	gBuffer->setTexture(RenderTargetBase::DEPTH, &GBDepth); //Z-BUFFER
	gBuffer->setTexture(RenderTargetBase::COLOR0, &GBColor0); //COLOR
	gBuffer->setTexture(RenderTargetBase::COLOR1, &GBColor1); //NORMAL, BRIGHTNESS, SPECULAR FACTOR

	SDepth.loadEmpty(vec3ui(4096,4096,NUM_SUN_CASCADES), TextureFormat::DEPTH_COMPONENT32F);
	SDepth.setFilter(GL_NEAREST, GL_NEAREST);
	SDepth.setComparison(GL_GREATER);
	sunTarget = new RenderTargetLayered(4096, 4096, NUM_SUN_CASCADES);
	sunTarget->setTexture(RenderTargetBase::DEPTH, &SDepth); //Z-BUFFER
	quad = Meshes.get("quad");
}

DeferredContainer::~DeferredContainer() {
	delete gBuffer;
	delete sunTarget;
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
	glEnable(GL_DEPTH_CLAMP);
	float shadowTime = Clock::getSeconds();
	drawMode = ShadowMap;
	RenderTargetBase::bind(sunTarget);
	GL_ASSERT(glClear(GL_DEPTH_BUFFER_BIT));
	ContainerObject::draw();
	glDisable(GL_DEPTH_CLAMP);
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
	const Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");
	Sun* sun = (Sun*)getGame()->getObjectByName("sun");
	if(Keyboard::pressed(Keyboard::Q)) cam = sun->getGlobalCam(); //sun cam mode
	glm::mat4 biasMatrix( //gets coords from [-1..1] to [0..1]
						  0.5, 0.0, 0.0, 0.0,
						  0.0, 0.5, 0.0, 0.0,
						  0.0, 0.0, 0.5, 0.0,
						  0.5, 0.5, 0.5, 1.0
						  );
	//compute each of the cascaded cameras's matrices
	std::vector<mat4f> depthMVP(NUM_SUN_CASCADES);
	for(int i = 0; i < NUM_SUN_CASCADES; ++i)
		depthMVP[i] = biasMatrix*(sun->getVPMatrices()[i]*fullTransform);
	Programs.get("ambientPass")->uniform("MVP")->set(mat4f(1.0f));
	Programs.get("ambientPass")->uniform("camMV")->set(cam->getView()*fullTransform);
	Programs.get("ambientPass")->uniform("color0")->set(getColor0());
	Programs.get("ambientPass")->uniform("color1")->set(getColor1());
	Programs.get("ambientPass")->uniform("invResolution")->set(vec2f(1.0f/screen->getSize().x, 1.0f/screen->getSize().y));
	Programs.get("ambientPass")->uniform("invCamProj")->set(glm::inverse(cam->projection));
	Programs.get("ambientPass")->uniform("invCamView")->set(glm::inverse(cam->getView()));
	Programs.get("ambientPass")->uniform("lightDir")->set(sun->getCam(0)->getForward());
	Programs.get("ambientPass")->uniform("worldsize")->set(WORLDSIZE);
	Programs.get("ambientPass")->uniform("depthMVP")->set(depthMVP);
	Programs.get("ambientPass")->uniform("depthPlanes")->set(sun->getDepthPlanes());
	Programs.get("ambientPass")->uniform("depth")->set(gBuffer->getTexture(RenderTargetBase::DEPTH));
	Programs.get("ambientPass")->uniform("sunDepth")->set(sunTarget->getTexture(RenderTargetBase::DEPTH));
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
	return gBuffer->getTexture(RenderTargetBase::COLOR0);
}

Texture2D *DeferredContainer::getColor1() const {
	return gBuffer->getTexture(RenderTargetBase::COLOR1);
}

Texture2D* DeferredContainer::getDepth() const {
	return gBuffer->getTexture(RenderTargetBase::DEPTH);
}

