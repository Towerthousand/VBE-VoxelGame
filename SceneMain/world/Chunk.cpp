#include "Chunk.hpp"
#include "World.hpp"

const int textureIndexes[9][6] = { //order is front, back, left, right, bottom, top
								   {0, 0, 0, 0, 0, 0}, //0 = air (empty, will never be used)
								   {2, 2, 2, 2, 2, 2}, //1 = dirt
								   {3, 3, 3, 3, 3, 3}, //2 = stone
								   {0, 0, 0, 0, 2, 1}, //3 = grass
								   {4, 4, 4, 4, 4, 4}, //4 = LIGHT
								   {5, 5, 5, 5, 5, 5}, //5 = cobble
								   {6, 6, 6, 6, 7, 7}, //6 = log
								   {8, 8, 8, 8, 8, 8}, //7 = planks
								   {9, 9, 9, 9, 9, 9}  //8 = sand
								 };

Chunk::Chunk(int x, unsigned int y, int z) : XPOS(x), YPOS(y), ZPOS(z), markedForRedraw(true), modelMatrix(mat4f(1.0f)), boundingBox(vec3f(0),vec3f(0)), world(nullptr) {
	if(Game::i() != nullptr) world = (World*)Game::i()->getObjectByName("World");
	modelMatrix = glm::translate(modelMatrix, vec3f(XPOS*CHUNKSIZE, YPOS*CHUNKSIZE, ZPOS*CHUNKSIZE));
	memset(cubes,0,sizeof(cubes));
}

Chunk::~Chunk() {
	delete terrainModel.mesh;
}

void Chunk::initMesh() {
	std::vector<Vertex::Element> elements = {
		Vertex::Element(Vertex::Attribute::Position, Vertex::Element::UnsignedByte, 3, Vertex::Element::ConvertToFloat),
		Vertex::Element(Vertex::Attribute::Normal, Vertex::Element::UnsignedByte, 1),
		Vertex::Element(Vertex::Attribute::TexCoord, Vertex::Element::UnsignedShort, 2, Vertex::Element::ConvertToFloat)
	};
	terrainModel.mesh = Mesh::loadEmpty(Vertex::Format(elements), Mesh::STATIC, false);
	terrainModel.program = Programs.get("deferredChunk");
	boundingBoxModel.mesh = Meshes.get("1x1Cube");
	boundingBoxModel.program = Programs.get("occlusionQuery");
}

unsigned int Chunk::getCube(int x, int y, int z) const { //in local space
	if(x >= 0 && x < CHUNKSIZE && y >= 0 && y < CHUNKSIZE && z >= 0 && z < CHUNKSIZE)
		return cubes[x][y][z];
	return world->getCube(x+(XPOS*CHUNKSIZE), y+(YPOS*CHUNKSIZE), z+(ZPOS*CHUNKSIZE)); //in another chunk
}

vec3i Chunk::getAbsolutePos() const {
	return vec3i(XPOS*CHUNKSIZE, YPOS*CHUNKSIZE, ZPOS*CHUNKSIZE);
}

AABB Chunk::getWorldSpaceBoundingBox() {
	return AABB(vec3f(modelMatrix*vec4f(boundingBox.getMin(),1)), vec3f(modelMatrix*vec4f(boundingBox.getMax(),1)));
}

void Chunk::update(float deltaTime) {
	(void) deltaTime;
	if(terrainModel.mesh == nullptr) initMesh();
	if(!markedForRedraw) return;
	markedForRedraw = false;
	std::vector<Chunk::Vert> renderData;
	bool first = true;
	boundingBox = AABB(vec3f(0), vec3f(0));
	for(int z = 0; z < CHUNKSIZE; ++z)
		for(int y = 0; y < CHUNKSIZE; ++y)
			for(int x = 0; x < CHUNKSIZE; ++x)
				if (cubes[x][y][z] != 0) { // only draw if it's not air
					unsigned int oldSize = renderData.size();
					pushCubeToArray(x, y, z, renderData);
					if(renderData.size() > oldSize){
						if(first) {
							boundingBox = AABB(vec3f(x, y, z), vec3f(x+1, y+1, z+1));
							first = false;
						}
						else {
							boundingBox.extend(vec3f(x, y, z));
							boundingBox.extend(vec3f(x+1, y+1, z+1));
						}
					}
				}
	terrainModel.mesh->setVertexData(&renderData[0], renderData.size());
}

void Chunk::draw() const {
	Camera* cam = world->getCamera();
	terrainModel.program->uniform("MVP")->set(cam->projection*cam->view*modelMatrix);
	terrainModel.program->uniform("M")->set(modelMatrix);
	terrainModel.program->uniform("V")->set(cam->view);
	terrainModel.program->uniform("diffuseTex")->set(Textures2D.get("blocks"));
	terrainModel.draw();
}

void Chunk::drawBoundingBox() const {
	if(boundingBox.getDimensions() == vec3f(0.0f)) return;
	Camera* cam = world->getCamera();
	boundingBoxModel.program->uniform("MVP")->set(cam->projection*cam->view*glm::scale(glm::translate(modelMatrix,boundingBox.getMin()), boundingBox.getDimensions()));
	boundingBoxModel.draw();
}

void Chunk::pushCubeToArray(short x, short y, short z, std::vector<Chunk::Vert> &renderData) { //I DON'T KNOW HOW TO MAKE THIS COMPACT
	short texY, texX;
	unsigned int cubeID = cubes[x][y][z];

	if(getCube(x, y, z+1) == 0) { // front face
		texX = (textureIndexes[cubeID][0] % (512/TEXSIZE))*TEXSIZE; // TEXSIZE/2 = number of textures/row
		texY = (textureIndexes[cubeID][0] / (512/TEXSIZE))*TEXSIZE; // TEXSIZE/2 = number of textures/row

		renderData.push_back(Chunk::Vert(x+1, y+1, z+1, 0, texX          , texY          ));
		renderData.push_back(Chunk::Vert(x  , y+1, z+1, 0, texX+TEXSIZE  , texY          ));
		renderData.push_back(Chunk::Vert(x+1, y  , z+1, 0, texX          , texY+TEXSIZE  ));

		renderData.push_back(Chunk::Vert(x  , y  , z+1, 0, texX+TEXSIZE  , texY+TEXSIZE  ));
		renderData.push_back(Chunk::Vert(x+1, y  , z+1, 0, texX          , texY+TEXSIZE  ));
		renderData.push_back(Chunk::Vert(x  , y+1, z+1, 0, texX+TEXSIZE  , texY          ));
	}
	if(getCube(x, y, z-1) == 0) { // back face
		texX = (textureIndexes[cubeID][1] % (512/TEXSIZE))*TEXSIZE;
		texY = (textureIndexes[cubeID][1] / (512/TEXSIZE))*TEXSIZE;

		renderData.push_back(Chunk::Vert(x+1, y  , z, 1, texX          , texY+TEXSIZE  ));
		renderData.push_back(Chunk::Vert(x  , y+1, z, 1, texX+TEXSIZE  , texY          ));
		renderData.push_back(Chunk::Vert(x+1, y+1, z, 1, texX          , texY          ));

		renderData.push_back(Chunk::Vert(x  , y  , z, 1, texX+TEXSIZE  , texY+TEXSIZE  ));
		renderData.push_back(Chunk::Vert(x  , y+1, z, 1, texX+TEXSIZE  , texY          ));
		renderData.push_back(Chunk::Vert(x+1, y  , z, 1, texX          , texY+TEXSIZE  ));
	}
	if(getCube(x+1, y, z) == 0) { // left face
		texX = (textureIndexes[cubeID][2] % (512/TEXSIZE))*TEXSIZE;
		texY = (textureIndexes[cubeID][2] / (512/TEXSIZE))*TEXSIZE;

		renderData.push_back(Chunk::Vert(x+1, y  , z+1, 2, texX        , texY+TEXSIZE  ));
		renderData.push_back(Chunk::Vert(x+1, y  , z  , 2, texX+TEXSIZE, texY+TEXSIZE  ));
		renderData.push_back(Chunk::Vert(x+1, y+1, z+1, 2, texX        , texY          ));

		renderData.push_back(Chunk::Vert(x+1, y  , z  , 2, texX+TEXSIZE, texY+TEXSIZE  ));
		renderData.push_back(Chunk::Vert(x+1, y+1, z  , 2, texX+TEXSIZE, texY));
		renderData.push_back(Chunk::Vert(x+1, y+1, z+1, 2, texX        , texY          ));
	}
	if(getCube(x-1, y, z) == 0) { // right face
		texX = (textureIndexes[cubeID][3] % (512/TEXSIZE))*TEXSIZE;
		texY = (textureIndexes[cubeID][3] / (512/TEXSIZE))*TEXSIZE;

		renderData.push_back(Chunk::Vert(x  , y  , z+1, 3, texX, texY+TEXSIZE          ));
		renderData.push_back(Chunk::Vert(x  , y+1, z+1, 3, texX        , texY          ));
		renderData.push_back(Chunk::Vert(x  , y  , z  , 3, texX+TEXSIZE, texY+TEXSIZE));

		renderData.push_back(Chunk::Vert(x  , y+1, z+1, 3, texX        , texY          ));
		renderData.push_back(Chunk::Vert(x  , y+1, z  , 3, texX+TEXSIZE, texY          ));
		renderData.push_back(Chunk::Vert(x  , y  , z  , 3, texX+TEXSIZE, texY+TEXSIZE			));
	}
	if(getCube(x, y-1, z) == 0) { // bottom face
		texX = (textureIndexes[cubeID][4] % (512/TEXSIZE))*TEXSIZE;
		texY = (textureIndexes[cubeID][4] / (512/TEXSIZE))*TEXSIZE;

		renderData.push_back(Chunk::Vert(x+1, y, z  , 4, texX+TEXSIZE  , texY  ));
		renderData.push_back(Chunk::Vert(x  , y, z+1, 4, texX  , texY+TEXSIZE  ));
		renderData.push_back(Chunk::Vert(x  , y, z  , 4, texX  , texY  ));

		renderData.push_back(Chunk::Vert(x+1, y, z  , 4, texX+TEXSIZE  , texY  ));
		renderData.push_back(Chunk::Vert(x+1, y, z+1, 4, texX+TEXSIZE    , texY+TEXSIZE    ));
		renderData.push_back(Chunk::Vert(x  , y, z+1, 4, texX  , texY+TEXSIZE  ));
	}
	if(getCube(x, y+1, z) == 0) { // top face
		texX = (textureIndexes[cubeID][5] % (512/TEXSIZE))*TEXSIZE;
		texY = (textureIndexes[cubeID][5] / (512/TEXSIZE))*TEXSIZE;

		renderData.push_back(Chunk::Vert(x+1, y+1, z  , 5, texX+TEXSIZE  , texY  ));
		renderData.push_back(Chunk::Vert(x  , y+1, z  , 5, texX  , texY  ));
		renderData.push_back(Chunk::Vert(x  , y+1, z+1, 5, texX  , texY+TEXSIZE  ));

		renderData.push_back(Chunk::Vert(x+1, y+1, z  , 5, texX+TEXSIZE  , texY  ));
		renderData.push_back(Chunk::Vert(x  , y+1, z+1, 5, texX  , texY+TEXSIZE  ));
		renderData.push_back(Chunk::Vert(x+1, y+1, z+1, 5, texX+TEXSIZE    , texY+TEXSIZE    ));
	}
}
