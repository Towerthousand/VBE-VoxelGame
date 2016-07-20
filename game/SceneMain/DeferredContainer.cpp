#include "DeferredContainer.hpp"
#include "BlurContainer.hpp"
#include "world/World.hpp"
#include "world/Chunk.hpp"
#include "world/Column.hpp"
#include "world/Sun.hpp"
#include "Manager.hpp"
#include "Debugger.hpp"

DeferredContainer::DeferredContainer() {
    setName("deferred");
    makeTarget();
    quad = &Meshes.get("quad");
}

DeferredContainer::~DeferredContainer() {
}

void DeferredContainer::update(float deltaTime) {
    makeTarget();
    ContainerObject::update(deltaTime);
}

void DeferredContainer::draw() const {
    //"The Screen". It may not be actually the screen since a upper container might be postprocessing
    const RenderTargetBase* screen = RenderTargetBase::getCurrent();

    GL_ASSERT(glEnable(GL_DEPTH_TEST));
    GL_ASSERT(glDisable(GL_BLEND));

    //Deferred pass
    Debugger::pushMark("Deferred Pass", "Time spent rendering geometry to the g-buffer");
    drawMode = Deferred;
    RenderTargetBase::bind(gBuffer);
    GL_ASSERT(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
    ContainerObject::draw();
    Debugger::popMark(); //deferred

    //Shadowmap pass
    Debugger::pushMark("Shadowmap Pass", "Time spent rendering geometry to the layered shadowmap");
    glEnable(GL_DEPTH_CLAMP);
    drawMode = ShadowMap;
    RenderTargetBase::bind(sunTarget);
    GL_ASSERT(glClear(GL_DEPTH_BUFFER_BIT));
    ContainerObject::draw();
    glDisable(GL_DEPTH_CLAMP);
    Debugger::popMark(); //shadow

    Debugger::pushMark("Light Pass", "Time spent rendering deferred lights");
    //bind output texture (screen)
    RenderTargetBase::bind(screen);
    GL_ASSERT(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

    //Light pass
    GL_ASSERT(glEnable(GL_BLEND));
    GL_ASSERT(glBlendFunc(GL_ONE, GL_ONE)); //additive
    GL_ASSERT(glDepthMask(GL_TRUE));
    GL_ASSERT(glDepthFunc(GL_ALWAYS));
    drawMode = Light;
    ContainerObject::draw();
    Debugger::popMark(); //lights

    //Ambient+Visibility pass
    Debugger::pushMark("Ambient+Visibility Pass", "Time spent rendering ambient light and sunlight contribution to the scene");
    const Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");
    Sun* sun = (Sun*)getGame()->getObjectByName("sun");
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
    Programs.get("ambientPass").uniform("MVP")->set(mat4f(1.0f));
    Programs.get("ambientPass").uniform("camMV")->set(cam->getView()*fullTransform);
    Programs.get("ambientPass").uniform("color0")->set(getColor0());
    Programs.get("ambientPass").uniform("color1")->set(getColor1());
    Programs.get("ambientPass").uniform("invResolution")->set(vec2f(1.0f/screen->getSize().x, 1.0f/screen->getSize().y));
    Programs.get("ambientPass").uniform("invCamProj")->set(glm::inverse(cam->projection));
    Programs.get("ambientPass").uniform("invCamView")->set(glm::inverse(cam->getView()));
    Programs.get("ambientPass").uniform("lightDir")->set(sun->getCam(0)->getForward());
    Programs.get("ambientPass").uniform("worldsize")->set(WORLDSIZE);
    Programs.get("ambientPass").uniform("depthMVP")->set(depthMVP);
    Programs.get("ambientPass").uniform("depthPlanes")->set(sun->getDepthPlanes());
    Programs.get("ambientPass").uniform("depth")->set(gBuffer.getTexture(RenderTargetBase::DEPTH));
    Programs.get("ambientPass").uniform("sunDepth")->set(sunTarget.getTexture(RenderTargetBase::DEPTH));
    quad->draw(Programs.get("ambientPass"));
    Debugger::popMark(); //ambient+shadowmap

    //Forward pass
    Debugger::pushMark("Forward Pass", "Time spent rendering forward-render stuff");
    GL_ASSERT(glDepthFunc(GL_LEQUAL));
    GL_ASSERT(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); //forward rendering blending
    drawMode = Forward;
    ContainerObject::draw();
    Debugger::popMark();
}

DeferredContainer::DrawMode DeferredContainer::getMode() const {
    return drawMode;
}

Texture2D *DeferredContainer::getColor0() const {
    return gBuffer.getTexture(RenderTargetBase::COLOR0);
}

Texture2D *DeferredContainer::getColor1() const {
    return gBuffer.getTexture(RenderTargetBase::COLOR1);
}

Texture2D* DeferredContainer::getDepth() const {
    return gBuffer.getTexture(RenderTargetBase::DEPTH);
}

void DeferredContainer::makeTarget() {
    if(Window::getInstance()->getSize() == gBuffer.getSize()) return;
    vec2ui size = Window::getInstance()->getSize();
    GBDepth = Texture2D(size, TextureFormat::DEPTH_COMPONENT32F);
    GBDepth.setFilter(GL_NEAREST, GL_NEAREST);
    GBColor0 = Texture2D(size, TextureFormat::RGBA16F);
    GBColor0.setFilter(GL_NEAREST, GL_NEAREST);
    GBColor1 = Texture2D(size, TextureFormat::RGBA16F);
    GBColor1.setFilter(GL_NEAREST, GL_NEAREST);
    gBuffer = RenderTarget(size.x, size.y);
    gBuffer.setTexture(RenderTargetBase::DEPTH, &GBDepth); //Z-BUFFER
    gBuffer.setTexture(RenderTargetBase::COLOR0, &GBColor0); //COLOR
    gBuffer.setTexture(RenderTargetBase::COLOR1, &GBColor1); //NORMAL, BRIGHTNESS, SPECULAR FACTOR
    if(sunTarget.getSize() == vec2ui(0)) { //invalid (this is the first makeTarget() )
        SDepth = Texture2DArray(vec3ui(4096,4096,NUM_SUN_CASCADES), TextureFormat::DEPTH_COMPONENT32F);
        SDepth.setFilter(GL_LINEAR, GL_LINEAR);
        SDepth.setComparison(GL_GREATER);
        sunTarget = RenderTargetLayered(4096, 4096, NUM_SUN_CASCADES);
        sunTarget.setTexture(RenderTargetBase::DEPTH, &SDepth); //Z-BUFFER
    }
}

