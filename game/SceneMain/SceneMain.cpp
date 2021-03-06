#include "SceneMain.hpp"
#include "Player.hpp"
#include "DeferredContainer.hpp"
#include "BlurContainer.hpp"
#include "world/World.hpp"
#include "world/DeferredCubeLight.hpp"
#include "Manager.hpp"
#include "Debugger.hpp"

SceneMain::SceneMain() {
    this->setName("SCENE");

    Window::getInstance()->setTitle("VoxelGame");
    Window::getInstance()->setVsync(Window::DisabledVsync);
    Mouse::setGrab(false);
    Mouse::setRelativeMode(true);

    loadResources();

    srand(Clock::getSeconds()*1000);

    //GL stuff..:
    GL_ASSERT(glClearColor(0, 0, 0, 1));
    GL_ASSERT(glEnable(GL_DEPTH_TEST));
    GL_ASSERT(glEnable(GL_BLEND));
    GL_ASSERT(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL_ASSERT(glDepthFunc(GL_LEQUAL));
    GL_ASSERT(glEnable(GL_CULL_FACE)); //enable backface culling
    GL_ASSERT(glCullFace(GL_BACK));
    GL_ASSERT(glEnable(GL_FRAMEBUFFER_SRGB));
    BlurContainer* blur = new BlurContainer();
    blur->addTo(this);

    DeferredContainer* renderer = new DeferredContainer();
    renderer->addTo(blur);

    World* world = new World();
    world->addTo(renderer);

    Player* player = new Player();
    player->addTo(this);

    Debugger* debug = new Debugger();
    debug->addTo(this);
}

SceneMain::~SceneMain() {
    Textures2D.clear();
    Meshes.clear();
    Programs.clear();
}

void SceneMain::loadResources() {
    //meshes
    std::vector<Vertex::Attribute> elems = {
        Vertex::Attribute("a_position", Vertex::Attribute::Float, 3)
    };
    std::vector<vec3f> data = {
        vec3f(1, -1, 0), vec3f(1, 1, 0), vec3f(-1, 1, 0),
        vec3f(-1, -1, 0)
    };
    std::vector<unsigned int> indexes = {
        0, 1, 2, 3, 0, 2
    };
    MeshIndexed quad = MeshIndexed(Vertex::Format(elems));
    quad.setVertexData(&data[0], 6);
    quad.setIndexData(&indexes[0], 6);
    quad.setPrimitiveType(Mesh::TRIANGLES);
    Meshes.add("quad", std::move(quad));

    std::vector<vec3f> cubeVertices = {
        vec3f(0.0, 0.0, 1.0),
        vec3f(1.0, 0.0, 1.0),
        vec3f(0.0, 1.0, 1.0),
        vec3f(1.0, 1.0, 1.0),
        vec3f(0.0, 0.0, 0.0),
        vec3f(1.0, 0.0, 0.0),
        vec3f(0.0, 1.0, 0.0),
        vec3f(1.0, 1.0, 0.0),
    };

    std::vector<unsigned int> cubeIndices = {
        0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1
    };

    MeshIndexed cube = MeshIndexed(Vertex::Format(elems));
    cube.setPrimitiveType(Mesh::TRIANGLE_STRIP);
    cube.setVertexData(&cubeVertices[0],cubeVertices.size());
    cube.setIndexData(&cubeIndices[0],cubeIndices.size());
    Meshes.add("1x1Cube", std::move(cube));

    //textures
    char pixels[4] = {char(200), char(20), char(20), char(255)};
    Texture2D nullRed(vec2ui(1));
    nullRed.setData(pixels);
    Textures2D.add("nullRed", std::move(nullRed));
    char pixels2[4] = {char(20), char(200), char(20), char(255)};
    Texture2D nullGreen = Texture2D(vec2ui(1));
    nullRed.setData(pixels2);
    Textures2D.add("nullGreen", std::move(nullGreen));
    char pixels3[4] = {char(20), char(20), char(200), char(255)};
    Texture2D nullBlue = Texture2D(vec2ui(1));
    nullRed.setData(pixels3);
    Textures2D.add("nullBlue", std::move(nullBlue));
    char pixels4[4] = {char(70), char(30), char(80), char(255)};
    Texture2D nullBlack = Texture2D(vec2ui(1));
    nullRed.setData(pixels4);
    Textures2D.add("nullBlack", std::move(nullBlack));
    char pixels5[4] = {char(255), char(255), char(255), char(255)};
    Texture2D nullWhite = Texture2D(vec2ui(1));
    nullRed.setData(pixels5);
    Textures2D.add("nullWhite", std::move(nullWhite));
    Textures2D.add("blocks", Texture2D::load(Storage::openAsset("textures/blocks8.png"), TextureFormat::SRGBA8));
    Textures2D.get("blocks").setFilter(GL_NEAREST,GL_NEAREST);

    //program
    Programs.add("deferredLight", ShaderProgram(Storage::openAsset("shaders/quad.vert"), Storage::openAsset("shaders/light.frag")));
    Programs.add("deferredCubeLight", ShaderProgram(Storage::openAsset("shaders/quad.vert"), Storage::openAsset("shaders/cubeLight.frag")));
    Programs.add("ambientPass", ShaderProgram(Storage::openAsset("shaders/quad.vert"), Storage::openAsset("shaders/ambientPass.frag")));
    Programs.add("blurPassVertical", ShaderProgram(Storage::openAsset("shaders/quad.vert"), Storage::openAsset("shaders/blurPassVertical.frag")));
    Programs.add("blurPassHoritzontal", ShaderProgram(Storage::openAsset("shaders/quad.vert"), Storage::openAsset("shaders/blurPassHoritzontal.frag")));
    Programs.add("textureToScreen", ShaderProgram(Storage::openAsset("shaders/quad.vert"), Storage::openAsset("shaders/quad.frag")));
    Programs.add("blurMaskPass", ShaderProgram(Storage::openAsset("shaders/quad.vert"), Storage::openAsset("shaders/blurMaskPass.frag")));
    Programs.add("depthShader", ShaderProgram(Storage::openAsset("shaders/depth.vert"), Storage::openAsset("shaders/depth.geom"), Storage::openAsset("shaders/depth.frag")));
    Programs.add("deferredChunk", ShaderProgram(Storage::openAsset("shaders/chunkDeferred.vert"), Storage::openAsset("shaders/chunkDeferred.frag")));
    Programs.add("forwardChunk", ShaderProgram(Storage::openAsset("shaders/chunkForward.vert"), Storage::openAsset("shaders/chunkForward.frag")));
}

void SceneMain::update(float deltaTime) {
    (void) deltaTime;
    if(Keyboard::pressed(Keyboard::Escape) || Window::getInstance()->isClosing()) getGame()->isRunning = false;
    if(Keyboard::justPressed(Keyboard::K)) USE_CPU_VISIBILITY = !USE_CPU_VISIBILITY;
}
