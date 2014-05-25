#include "Chunk.hpp"
#include "World.hpp"
#include "../DeferredContainer.hpp"

const int textureIndexes[9][6] = { //order is front, back, left, right, bottom, top
								   { 0,  0,  0,  0,  0,  0}, //0 = air (empty, will never be used)
								   { 1,  1,  1,  1,  1,  1}, //1 = stone
								   { 3,  3,  3,  3,  2,  0}, //2 = grass
								   { 2,  2,  2,  2,  2,  2}, //3 = dirt
								   {16, 16, 16, 16, 16, 16}, //4 = cobble
								   { 4,  4,  4,  4,  4,  4}, //5 = planks
								   { 0,  0,  0,  0,  0,  0}, //6 = sapling
								   {17, 17, 17, 17, 17, 17}, //7 = bedrock
								   {14, 14, 14, 14, 14, 14}  //8 = flowing water
								 };

Chunk::Chunk(int x, unsigned int y, int z) : XPOS(x), YPOS(y), ZPOS(z), markedForRedraw(true), modelMatrix(mat4f(1.0f)), boundingBox(vec3f(0),vec3f(0)), world(nullptr), renderer(nullptr) {
	if(Game::i() != nullptr) {
		world = (World*)Game::i()->getObjectByName("world");
		renderer = (DeferredContainer*)Game::i()->getObjectByName("deferred");
	}
	modelMatrix = glm::translate(modelMatrix, vec3f(XPOS*CHUNKSIZE, YPOS*CHUNKSIZE, ZPOS*CHUNKSIZE));
	initMesh();
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

unsigned int Chunk::getBlock(int x, int y, int z) const {
	return world->getBlock(x+(XPOS*CHUNKSIZE), y+(YPOS*CHUNKSIZE), z+(ZPOS*CHUNKSIZE));
}

vec3i Chunk::getAbsolutePos() const {
	return vec3i(XPOS*CHUNKSIZE, YPOS*CHUNKSIZE, ZPOS*CHUNKSIZE);
}

AABB Chunk::getWorldSpaceBoundingBox() {
	return AABB(vec3f(modelMatrix*vec4f(boundingBox.getMin(),1)), vec3f(modelMatrix*vec4f(boundingBox.getMax(),1)));
}

void Chunk::update(float deltaTime) {
	(void) deltaTime;
	if(!markedForRedraw) return;
	markedForRedraw = false;
	std::vector<Chunk::Vert> renderData;
	boundingBox = AABB();
	for(int z = 0; z < CHUNKSIZE; ++z)
		for(int y = 0; y < CHUNKSIZE; ++y)
			for(int x = 0; x < CHUNKSIZE; ++x)
				if (getBlock(x, y, z) != 0) { // only draw if it's not air
					unsigned int oldSize = renderData.size();
					pushCubeToArray(x, y, z, renderData);
					if(renderData.size() > oldSize){
						boundingBox.extend(vec3f(x, y, z));
						boundingBox.extend(vec3f(x+1, y+1, z+1));
					}
				}
	terrainModel.mesh->setVertexData(&renderData[0], renderData.size());
}

void Chunk::draw() const {
	if(renderer->getMode() == DeferredContainer::Deferred) {
		terrainModel.program = Programs.get("deferredChunk");
		Camera* cam = (Camera*)Game::i()->getObjectByName("playerCam");
		terrainModel.program->uniform("MVP")->set(cam->projection*cam->getView()*modelMatrix);
		terrainModel.program->uniform("M")->set(modelMatrix);
		terrainModel.program->uniform("V")->set(cam->getView());
		terrainModel.program->uniform("diffuseTex")->set(Textures2D.get("blocks"));
		terrainModel.draw();
	}
	else if(renderer->getMode() == DeferredContainer::ShadowMap) {
		terrainModel.program = Programs.get("occlusionQuery");
		Camera* cam = (Camera*)Game::i()->getObjectByName("sunCamera");
		terrainModel.program->uniform("MVP")->set(cam->projection*cam->getView()*modelMatrix);
		terrainModel.draw();
	}
}

void Chunk::drawBoundingBox() const {
	if(boundingBox.getDimensions() == vec3f(0.0f)) return;
	if(renderer->getMode() == DeferredContainer::Deferred) {
		Camera* cam = (Camera*)Game::i()->getObjectByName("playerCam");
		boundingBoxModel.program->uniform("MVP")->set(cam->projection*cam->getView()*glm::scale(glm::translate(modelMatrix,boundingBox.getMin()), boundingBox.getDimensions()));
		boundingBoxModel.draw();
	}
	else if(renderer->getMode() == DeferredContainer::ShadowMap) {
		Camera* cam = (Camera*)Game::i()->getObjectByName("sunCamera");
		boundingBoxModel.program->uniform("MVP")->set(cam->projection*cam->getView()*glm::scale(glm::translate(modelMatrix,boundingBox.getMin()), boundingBox.getDimensions()));
		boundingBoxModel.draw();
	}
}

void Chunk::pushCubeToArray(short x, short y, short z, std::vector<Chunk::Vert> &renderData) { //I DON'T KNOW HOW TO MAKE THIS COMPACT
	short texY, texX;
	unsigned int cubeID = getBlock(x, y, z);
	const int textureTiles = 16; //Texture is textureTiles*textureTiles, we don't care about the pixel size.
	const int tileSize = (1<<9)/textureTiles;

	if(getBlock(x, y, z+1) == 0) { // front face
		texX = (textureIndexes[cubeID][0] % (textureTiles))*tileSize;
		texY = (textureIndexes[cubeID][0] / (textureTiles))*tileSize;

		renderData.push_back(Chunk::Vert(x+1, y+1, z+1, 0, texX+tileSize          , texY          ));
		renderData.push_back(Chunk::Vert(x  , y+1, z+1, 0, texX , texY          ));
		renderData.push_back(Chunk::Vert(x+1, y  , z+1, 0, texX+tileSize          , texY+tileSize ));

		renderData.push_back(Chunk::Vert(x  , y  , z+1, 0, texX , texY+tileSize ));
		renderData.push_back(Chunk::Vert(x+1, y  , z+1, 0, texX+tileSize          , texY+tileSize ));
		renderData.push_back(Chunk::Vert(x  , y+1, z+1, 0, texX , texY          ));
	}
	if(getBlock(x, y, z-1) == 0) { // back face
		texX = (textureIndexes[cubeID][1] % (textureTiles))*tileSize;
		texY = (textureIndexes[cubeID][1] / (textureTiles))*tileSize;

		renderData.push_back(Chunk::Vert(x+1, y  , z, 1, texX          , texY+tileSize ));
		renderData.push_back(Chunk::Vert(x  , y+1, z, 1, texX+tileSize , texY          ));
		renderData.push_back(Chunk::Vert(x+1, y+1, z, 1, texX          , texY          ));

		renderData.push_back(Chunk::Vert(x  , y  , z, 1, texX+tileSize , texY+tileSize ));
		renderData.push_back(Chunk::Vert(x  , y+1, z, 1, texX+tileSize , texY          ));
		renderData.push_back(Chunk::Vert(x+1, y  , z, 1, texX          , texY+tileSize ));
	}
	if(getBlock(x+1, y, z) == 0) { // left face
		texX = (textureIndexes[cubeID][2] % (textureTiles))*tileSize;
		texY = (textureIndexes[cubeID][2] / (textureTiles))*tileSize;

		renderData.push_back(Chunk::Vert(x+1, y  , z+1, 2, texX        , texY+tileSize ));
		renderData.push_back(Chunk::Vert(x+1, y  , z  , 2, texX+tileSize, texY+tileSize ));
		renderData.push_back(Chunk::Vert(x+1, y+1, z+1, 2, texX        , texY          ));

		renderData.push_back(Chunk::Vert(x+1, y  , z  , 2, texX+tileSize, texY+tileSize ));
		renderData.push_back(Chunk::Vert(x+1, y+1, z  , 2, texX+tileSize, texY));
		renderData.push_back(Chunk::Vert(x+1, y+1, z+1, 2, texX        , texY          ));
	}
	if(getBlock(x-1, y, z) == 0) { // right face
		texX = (textureIndexes[cubeID][3] % (textureTiles))*tileSize;
		texY = (textureIndexes[cubeID][3] / (textureTiles))*tileSize;

		renderData.push_back(Chunk::Vert(x  , y  , z+1, 3, texX+tileSize, texY+tileSize         ));
		renderData.push_back(Chunk::Vert(x  , y+1, z+1, 3, texX+tileSize        , texY          ));
		renderData.push_back(Chunk::Vert(x  , y  , z  , 3, texX, texY+tileSize));

		renderData.push_back(Chunk::Vert(x  , y+1, z+1, 3, texX+tileSize        , texY          ));
		renderData.push_back(Chunk::Vert(x  , y+1, z  , 3, texX , texY          ));
		renderData.push_back(Chunk::Vert(x  , y  , z  , 3, texX , texY+tileSize			));
	}
	if(getBlock(x, y-1, z) == 0) { // bottom face
		texX = (textureIndexes[cubeID][4] % (textureTiles))*tileSize;
		texY = (textureIndexes[cubeID][4] / (textureTiles))*tileSize;

		renderData.push_back(Chunk::Vert(x+1, y, z  , 4, texX , texY  ));
		renderData.push_back(Chunk::Vert(x  , y, z+1, 4, texX+tileSize  , texY+tileSize ));
		renderData.push_back(Chunk::Vert(x  , y, z  , 4, texX+tileSize  , texY  ));

		renderData.push_back(Chunk::Vert(x+1, y, z  , 4, texX , texY  ));
		renderData.push_back(Chunk::Vert(x+1, y, z+1, 4, texX   , texY+tileSize   ));
		renderData.push_back(Chunk::Vert(x  , y, z+1, 4, texX+tileSize  , texY+tileSize ));
	}
	if(getBlock(x, y+1, z) == 0) { // top face
		texX = (textureIndexes[cubeID][5] % (textureTiles))*tileSize;
		texY = (textureIndexes[cubeID][5] / (textureTiles))*tileSize;

		renderData.push_back(Chunk::Vert(x+1, y+1, z  , 5, texX+tileSize , texY  ));
		renderData.push_back(Chunk::Vert(x  , y+1, z  , 5, texX  , texY  ));
		renderData.push_back(Chunk::Vert(x  , y+1, z+1, 5, texX  , texY+tileSize ));

		renderData.push_back(Chunk::Vert(x+1, y+1, z  , 5, texX+tileSize , texY  ));
		renderData.push_back(Chunk::Vert(x  , y+1, z+1, 5, texX  , texY+tileSize ));
		renderData.push_back(Chunk::Vert(x+1, y+1, z+1, 5, texX+tileSize   , texY+tileSize   ));
	}
	return;
}
