#include "SceneMain.hpp"
#include "Player.hpp"
#include "DeferredContainer.hpp"
#include "DeferredLight.hpp"
#include "BlurContainer.hpp"
#include "world/World.hpp"
#include "world/DeferredCubeLight.hpp"

SceneMain::SceneMain() : debugCounter(0.0), fpsCount(0) {
	this->setName("SCENE");

	loadResources();

	//GL stuff..:
	GL_ASSERT(glClearColor(0, 0, 0, 1));
	GL_ASSERT(glEnable(GL_DEPTH_TEST));
	GL_ASSERT(glEnable(GL_BLEND));
	GL_ASSERT(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GL_ASSERT(glDepthFunc(GL_LEQUAL));
	GL_ASSERT(glEnable(GL_CULL_FACE)); //enable backface culling
	GL_ASSERT(glCullFace(GL_BACK));

	//getGame()->getWindow().setVerticalSyncEnabled(true);

	BlurContainer* blur = new BlurContainer();
	blur->addTo(this);

	DeferredContainer* renderer = new DeferredContainer();
	renderer->addTo(blur);

	World* world = new World();
	world->addTo(renderer);

	Player* player = new Player();
	player->getCam()->projection = glm::perspective(60.0f, float(Environment::getScreen()->getWidth())/float(Environment::getScreen()->getHeight()), 0.01f, 100.0f);
	//player->getCam()->projection = glm::ortho(-5.0f,5.0f,-5.0f,5.0f,-100.0f, 100.0f);
	player->addTo(this);
}

SceneMain::~SceneMain() {
	Textures2D.clear();
	Meshes.clear();
	Programs.clear();
}

void SceneMain::loadResources() {
	//meshes
	std::vector<Vertex::Element> elems = {
		Vertex::Element(Vertex::Element(Vertex::Attribute::Position, Vertex::Element::Float, 3))
	};
	std::vector<vec3f> data = {
		vec3f(1, -1, 0), vec3f(1, 1, 0), vec3f(-1, 1, 0),
		vec3f(-1, 1, 0), vec3f(-1, -1, 0), vec3f(1, -1, 0)
	};
	Mesh* quad = Mesh::loadEmpty(Vertex::Format(elems));
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

	Mesh* cube = Mesh::loadEmpty(Vertex::Format(elems),Mesh::STATIC,true);
	cube->setPrimitiveType(Mesh::TRIANGLE_STRIP);
	cube->setVertexData(&cubeVertices[0],cubeVertices.size());
	cube->setVertexIndices(&cubeIndices[0],cubeIndices.size());
	Meshes.add("1x1Cube",cube);

	//textures
	char pixels[4] = {char(200), char(20), char(20), char(255)};
	Textures2D.add("nullRed", Texture2D::createFromRaw(pixels, 1, 1));
	char pixels2[4] = {char(20), char(200), char(20), char(255)};
	Textures2D.add("nullGreen", Texture2D::createFromRaw(pixels2, 1, 1));
	char pixels3[4] = {char(20), char(20), char(200), char(255)};
	Textures2D.add("nullBlue", Texture2D::createFromRaw(pixels3, 1, 1));
	char pixels4[4] = {char(70), char(30), char(80), char(255)};
	Textures2D.add("nullBlack", Texture2D::createFromRaw(pixels4, 1, 1));
	char pixels5[4] = {char(255), char(255), char(255), char(255)};
	Textures2D.add("nullWhite", Texture2D::createFromRaw(pixels5, 1, 1));
	Textures2D.add("blocks",Texture2D::createFromFile("data/textures/blocks8.png"));
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
	Programs.add("occlusionQuery", ShaderProgram::loadFromFile("data/shaders/occlusionQuery.vert", "data/shaders/occlusionQuery.frag"));
}

void SceneMain::update(float deltaTime) {
	++fpsCount;
	debugCounter += deltaTime;
	if (debugCounter > 1) {
		VBE_LOG("FPS: " << fpsCount);
		debugCounter--;
		fpsCount = 0;
	}
	if(Environment::getKeyboard()->isKeyPressed(Keyboard::Escape)) getGame()->isRunning = false;
}
