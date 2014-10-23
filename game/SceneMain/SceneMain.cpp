#include "SceneMain.hpp"
#include "Player.hpp"
#include "DeferredContainer.hpp"
#include "DeferredLight.hpp"
#include "BlurContainer.hpp"
#include "world/World.hpp"
#include "world/DeferredCubeLight.hpp"
#include "debug/Profiler.hpp"
#include "Manager.hpp"

SceneMain::SceneMain() {
	this->setName("SCENE");

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
	player->getCam()->projection = glm::perspective(60.0f, float(Window::getInstance()->getSize().x)/float(Window::getInstance()->getSize().y), 0.01f, WORLDSIZE*CHUNKSIZE*0.5f*1.735f);
	player->addTo(this);

	Profiler* debug = new Profiler();
	debug->addTo(this);
}

SceneMain::~SceneMain() {
	Textures2D.clear();
	Meshes.clear();
	Programs.clear();
}

void SceneMain::loadResources() {
	//meshes
	std::vector<Vertex::Element> elems = {
		Vertex::Element(Vertex::Attribute::Position, Vertex::Element::Float, 3)
	};
	std::vector<vec3f> data = {
		vec3f(1, -1, 0), vec3f(1, 1, 0), vec3f(-1, 1, 0),
		vec3f(-1, 1, 0), vec3f(-1, -1, 0), vec3f(1, -1, 0)
	};
	Mesh* quad = new Mesh(Vertex::Format(elems));
	quad->setVertexData(&data[0], 6);
	quad->setPrimitiveType(Mesh::TRIANGLES);
	Meshes.add("quad", quad);

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

	MeshIndexed* cube = new MeshIndexed(Vertex::Format(elems));
	cube->setPrimitiveType(Mesh::TRIANGLE_STRIP);
	cube->setVertexData(&cubeVertices[0],cubeVertices.size());
	cube->setIndexData(&cubeIndices[0],cubeIndices.size());
	Meshes.add("1x1Cube",cube);

	//textures
	char pixels[4] = {char(200), char(20), char(20), char(255)};
	Texture2D* nullRed = new Texture2D();
	nullRed->loadFromRaw(pixels, vec2ui(1));
	Textures2D.add("nullRed", nullRed);
	char pixels2[4] = {char(20), char(200), char(20), char(255)};
	Texture2D* nullGreen = new Texture2D();
	nullRed->loadFromRaw(pixels2, vec2ui(1));
	Textures2D.add("nullGreen", nullGreen);
	char pixels3[4] = {char(20), char(20), char(200), char(255)};
	Texture2D* nullBlue = new Texture2D();
	nullRed->loadFromRaw(pixels3, vec2ui(1));
	Textures2D.add("nullBlue", nullBlue);
	char pixels4[4] = {char(70), char(30), char(80), char(255)};
	Texture2D* nullBlack = new Texture2D();
	nullRed->loadFromRaw(pixels4, vec2ui(1));
	Textures2D.add("nullBlack", nullBlack);
	char pixels5[4] = {char(255), char(255), char(255), char(255)};
	Texture2D* nullWhite = new Texture2D();
	nullRed->loadFromRaw(pixels5, vec2ui(1));
	Textures2D.add("nullWhite", nullWhite);
	Texture2D* blocks = new Texture2D();
	blocks->loadFromFile("data/textures/blocks8.png", TextureFormat::SRGBA8);
	Textures2D.add("blocks", blocks);
	Textures2D.get("blocks")->setFilter(GL_NEAREST,GL_NEAREST);

	//program
	Programs.add("deferredLight", ShaderProgram::loadFromFile("data/shaders/quad.vert", "data/shaders/light.frag"));
	Programs.add("deferredCubeLight", ShaderProgram::loadFromFile("data/shaders/quad.vert", "data/shaders/cubeLight.frag"));
	Programs.add("ambientPass", ShaderProgram::loadFromFile("data/shaders/quad.vert", "data/shaders/ambientPass.frag"));
	Programs.add("blurPassVertical", ShaderProgram::loadFromFile("data/shaders/quad.vert", "data/shaders/blurPassVertical.frag"));
	Programs.add("blurPassHoritzontal", ShaderProgram::loadFromFile("data/shaders/quad.vert", "data/shaders/blurPassHoritzontal.frag"));
	Programs.add("textureToScreen", ShaderProgram::loadFromFile("data/shaders/quad.vert", "data/shaders/quad.frag"));
	Programs.add("blurMaskPass", ShaderProgram::loadFromFile("data/shaders/quad.vert", "data/shaders/blurMaskPass.frag"));
	Programs.add("depthShader", ShaderProgram::loadFromFile("data/shaders/depth.vert","data/shaders/depth.frag"));
	Programs.add("deferredChunk", ShaderProgram::loadFromFile("data/shaders/chunkDeferred.vert", "data/shaders/chunkDeferred.frag"));
	Programs.add("debugDraw", ShaderProgram::loadFromFile("data/shaders/debugDraw.vert", "data/shaders/debugDraw.frag"));
}

void SceneMain::update(float deltaTime) {
	(void) deltaTime;
	if(Keyboard::pressed(Keyboard::Escape) || Window::getInstance()->isClosing()) getGame()->isRunning = false;
}
