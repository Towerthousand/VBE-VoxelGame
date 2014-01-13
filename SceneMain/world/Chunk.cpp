#include "Chunk.hpp"
#include "World.hpp"
#include "../Camera.hpp"

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

Chunk::Chunk(int x, unsigned int y, int z) : XPOS(x), YPOS(y), ZPOS(z), vertexCount(0), modelMatrix(mat4f(1.0f)), markedForRedraw(true), world((World*)Game::i()->getObjectByName("World")) {
	modelMatrix = glm::translate(modelMatrix, vec3f(XPOS*CHUNKSIZE, YPOS*CHUNKSIZE, ZPOS*CHUNKSIZE));
	modelMatrix = glm::scale(modelMatrix, vec3f(0.5, 0.5, 0.5));
	model.program = Programs.get("deferredChunk");
	std::vector<Vertex::Element> elements = {
		Vertex::Element(Vertex::Attribute::Position, Vertex::Element::UnsignedByte, 3, Vertex::Element::ConvertToFloat),
		Vertex::Element(Vertex::Attribute::Normal, Vertex::Element::UnsignedByte, 1),
		Vertex::Element(Vertex::Attribute::TexCoord, Vertex::Element::UnsignedShort, 2, Vertex::Element::ConvertToFloat),
		Vertex::Element(Vertex::Attribute::Color, Vertex::Element::UnsignedByte, 4, Vertex::Element::ConvertToFloatNormalized)
	};
	model.mesh = Mesh::loadEmpty(Vertex::Format(elements), Mesh::STATIC, false);
	boundingBox.mesh = Meshes.get("1x1Cube");
	boundingBox.program = Programs.get("occlusionQuery");
}

Chunk::~Chunk() {
	delete model.mesh;
}

Cube Chunk::getCube(int x, int y, int z) const { //in local space
	if(x >= 0 && x < CHUNKSIZE && y >= 0 && y < CHUNKSIZE && z >= 0 && z < CHUNKSIZE)
		return cubes[x][y][z];
	return world->getCube(x+(XPOS*CHUNKSIZE), y+(YPOS*CHUNKSIZE), z+(ZPOS*CHUNKSIZE)); //in another chunk
}

vec3i Chunk::getAbsolutePos() {
	return vec3i(XPOS*CHUNKSIZE, YPOS*CHUNKSIZE, ZPOS*CHUNKSIZE);
}

void Chunk::update(float deltaTime) {
	(void) deltaTime;
	if(!markedForRedraw) return;
	markedForRedraw = false;
	std::vector<Chunk::Vert> renderData;
	for(int z = 0; z < CHUNKSIZE; ++z)
		for(int y = 0; y < CHUNKSIZE; ++y)
			for(int x = 0; x < CHUNKSIZE; ++x)
				if (cubes[x][y][z].ID != 0)  // only draw if it's not air
					pushCubeToArray(x, y, z, renderData);
	model.mesh->setVertexData(&renderData[0], renderData.size());
	vertexCount = renderData.size();
}

void Chunk::draw() const {
	Camera* cam = world->getCamera();
	model.program->uniform("MVP")->set(cam->projection*cam->view*modelMatrix);
	model.program->uniform("M")->set(modelMatrix);
	model.program->uniform("V")->set(cam->view);
	model.program->uniform("diffuseTex")->set(Textures.get("blocks"));
	model.draw();
}

void Chunk::drawBoundingBox() const {
	Camera* cam = world->getCamera();
	boundingBox.program->uniform("MVP")->set(cam->projection*cam->view*glm::scale(modelMatrix, vec3f(CHUNKSIZE*2)));
	boundingBox.draw();
}

void Chunk::pushCubeToArray(short x, short y, short z, std::vector<Chunk::Vert> &renderData) { //I DON'T KNOW HOW TO MAKE THIS COMPACT
	short absX = 2*x;
	short absY = 2*y;
	short absZ = 2*z;
	short texY, texX;
	float lindAf = 1.0, lindBf = 1.0, lindCf = 1.0, lindDf = 1.0;
	unsigned char lindA = 255, lindB = 255, lindC = 255, lindD = 255, lindE = 255;
	unsigned char cubeID = cubes[x][y][z].ID;
	//STRUCTURE PER VERTEX: Vx, Vy, Vz,
	//						Tx, Ty,
	//						Cr, Cg, Cb, Ca
	if(getCube(x, y, z+1).ID == 0) { // front face
		if (cubeID != 4) {
			//if it's not a light (light should be fully lit) calculate the average of the adjacent
			//air blocks and assign max(max(average, adjacentBlock.light/2), MINLIGHT)
			unsigned char centerLight = getCube(x, y, z+1).light;
			lindAf = (centerLight + getCube(x, y+1, z+1).light +
					  getCube(x-1, y, z+1).light + getCube(x-1, y+1, z+1).light)/4.0; //between 0 and MAXLIGHT
			lindBf = (centerLight + getCube(x, y-1, z+1).light +
					  getCube(x-1, y, z+1).light + getCube(x-1, y-1, z+1).light)/4.0;
			lindCf = (centerLight + getCube(x, y-1, z+1).light +
					  getCube(x+1, y, z+1).light + getCube(x+1, y-1, z+1).light)/4.0;
			lindDf = (centerLight + getCube(x, y+1, z+1).light +
					  getCube(x+1, y, z+1).light + getCube(x+1, y+1, z+1).light)/4.0;
			lindA = (std::fmax(std::fmax(lindAf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindB = (std::fmax(std::fmax(lindBf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindC = (std::fmax(std::fmax(lindCf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindD = (std::fmax(std::fmax(lindDf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindE = (lindA+lindB+lindC+lindD)/4; //between 0 and 255
		}
		texX = (textureIndexes[cubeID][0] % (512/TEXSIZE))*TEXSIZE; // TEXSIZE/2 = number of textures/row
		texY = (textureIndexes[cubeID][0] / (512/TEXSIZE))*TEXSIZE; // TEXSIZE/2 = number of textures/row
		//t1
		renderData.push_back(Chunk::Vert(absX+2, absY+2, absZ+2, 0, texX          , texY          , lindD, lindD, lindD, 255));
		renderData.push_back(Chunk::Vert(absX  , absY+2, absZ+2, 0, texX+TEXSIZE  , texY          , lindA, lindA, lindA, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY+1, absZ+2, 0, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t2
		renderData.push_back(Chunk::Vert(absX+2, absY  , absZ+2, 0, texX	      , texY+TEXSIZE  , lindC, lindC, lindC, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY+2, absZ+2, 0, texX          , texY          , lindD, lindD, lindD, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY+1, absZ+2, 0, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t3
		renderData.push_back(Chunk::Vert(absX  , absY  , absZ+2, 0, texX+TEXSIZE  , texY+TEXSIZE  , lindB, lindB, lindB, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY  , absZ+2, 0, texX	      , texY+TEXSIZE  , lindC, lindC, lindC, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY+1, absZ+2, 0, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t4
		renderData.push_back(Chunk::Vert(absX  , absY+2, absZ+2, 0, texX+TEXSIZE  , texY          , lindA, lindA, lindA, 255));
		renderData.push_back(Chunk::Vert(absX  , absY  , absZ+2, 0, texX+TEXSIZE  , texY+TEXSIZE  , lindB, lindB, lindB, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY+1, absZ+2, 0, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
	}
	if(getCube(x, y, z-1).ID == 0) { // back face
		if (cubeID != 4) {
			unsigned char centerLight = getCube(x, y, z-1).light;
			lindAf = (centerLight + getCube(x, y+1, z-1).light +
					  getCube(x+1, y, z-1).light + getCube(x+1, y+1, z-1).light)/4.0;
			lindBf = (centerLight + getCube(x, y-1, z-1).light +
					  getCube(x+1, y, z-1).light + getCube(x+1, y-1, z-1).light)/4.0;
			lindCf = (centerLight + getCube(x, y-1, z-1).light +
					  getCube(x-1, y, z-1).light + getCube(x-1, y-1, z-1).light)/4.0;
			lindDf = (centerLight + getCube(x, y+1, z-1).light +
					  getCube(x-1, y, z-1).light + getCube(x-1, y+1, z-1).light)/4.0;
			lindA = (std::fmax(std::fmax(lindAf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindB = (std::fmax(std::fmax(lindBf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindC = (std::fmax(std::fmax(lindCf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindD = (std::fmax(std::fmax(lindDf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindE = (lindA+lindB+lindC+lindD)/4;
		}
		texX = (textureIndexes[cubeID][1] % (512/TEXSIZE))*TEXSIZE;
		texY = (textureIndexes[cubeID][1] / (512/TEXSIZE))*TEXSIZE;
		//t1
		renderData.push_back(Chunk::Vert(absX  , absY+2, absZ, 1, texX+TEXSIZE  , texY          , lindD, lindD, lindD, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY+2, absZ, 1, texX          , texY          , lindA, lindA, lindA, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY+1, absZ, 1, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t2
		renderData.push_back(Chunk::Vert(absX+2, absY+2, absZ, 1, texX          , texY          , lindA, lindA, lindA, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY  , absZ, 1, texX	        , texY+TEXSIZE  , lindB, lindB, lindB, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY+1, absZ, 1, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t3
		renderData.push_back(Chunk::Vert(absX+2, absY  , absZ, 1, texX	        , texY+TEXSIZE  , lindB, lindB, lindB, 255));
		renderData.push_back(Chunk::Vert(absX  , absY  , absZ, 1, texX+TEXSIZE  , texY+TEXSIZE  , lindC, lindC, lindC, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY+1, absZ, 1, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t4
		renderData.push_back(Chunk::Vert(absX  , absY  , absZ, 1, texX+TEXSIZE  , texY+TEXSIZE  , lindC, lindC, lindC, 255));
		renderData.push_back(Chunk::Vert(absX  , absY+2, absZ, 1, texX+TEXSIZE  , texY          , lindD, lindD, lindD, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY+1, absZ, 1, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
	}
	if(getCube(x+1, y, z).ID == 0) { // left face
		if (cubeID != 4) {
			unsigned char centerLight = getCube(x+1, y, z).light;
			lindAf = (centerLight + getCube(x+1, y+1, z).light +
					  getCube(x+1, y, z+1).light + getCube(x+1, y+1, z+1).light)/4.0;
			lindBf = (centerLight + getCube(x+1, y-1, z).light +
					  getCube(x+1, y, z+1).light + getCube(x+1, y-1, z+1).light)/4.0;
			lindCf = (centerLight + getCube(x+1, y-1, z).light +
					  getCube(x+1, y, z-1).light + getCube(x+1, y-1, z-1).light)/4.0;
			lindDf = (centerLight + getCube(x+1, y+1, z).light +
					  getCube(x+1, y, z-1).light + getCube(x+1, y+1, z-1).light)/4.0;
			lindA = (std::fmax(std::fmax(lindAf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindB = (std::fmax(std::fmax(lindBf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindC = (std::fmax(std::fmax(lindCf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindD = (std::fmax(std::fmax(lindDf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindE = (lindA+lindB+lindC+lindD)/4;
		}
		texX = (textureIndexes[cubeID][2] % (512/TEXSIZE))*TEXSIZE;
		texY = (textureIndexes[cubeID][2] / (512/TEXSIZE))*TEXSIZE;
		//t1
		renderData.push_back(Chunk::Vert(absX+2, absY+2, absZ  , 2, texX          , texY          , lindD, lindD, lindD, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY+2, absZ+2, 2, texX+TEXSIZE  , texY          , lindA, lindA, lindA, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY+1, absZ+1, 2, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t2
		renderData.push_back(Chunk::Vert(absX+2, absY  , absZ  , 2, texX          , texY+TEXSIZE  , lindC, lindC, lindC, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY+2, absZ  , 2, texX          , texY          , lindD, lindD, lindD, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY+1, absZ+1, 2, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t3
		renderData.push_back(Chunk::Vert(absX+2, absY  , absZ+2, 2, texX+TEXSIZE  , texY+TEXSIZE  , lindB, lindB, lindB, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY  , absZ  , 2, texX          , texY+TEXSIZE  , lindC, lindC, lindC, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY+1, absZ+1, 2, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t4
		renderData.push_back(Chunk::Vert(absX+2, absY+2, absZ+2, 2, texX+TEXSIZE  , texY          , lindA, lindA, lindA, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY  , absZ+2, 2, texX+TEXSIZE  , texY+TEXSIZE  , lindB, lindB, lindB, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY+1, absZ+1, 2, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
	}
	if(getCube(x-1, y, z).ID == 0) { // right face
		if (cubeID != 4) {
			unsigned char centerLight = getCube(x-1, y, z).light;
			lindAf = (centerLight + getCube(x-1, y+1, z).light +
					  getCube(x-1, y, z-1).light + getCube(x-1, y+1, z-1).light)/4.0;
			lindBf = (centerLight + getCube(x-1, y-1, z).light +
					  getCube(x-1, y, z-1).light + getCube(x-1, y-1, z-1).light)/4.0;
			lindCf = (centerLight + getCube(x-1, y-1, z).light +
					  getCube(x-1, y, z+1).light + getCube(x-1, y-1, z+1).light)/4.0;
			lindDf = (centerLight + getCube(x-1, y+1, z).light +
					  getCube(x-1, y, z+1).light + getCube(x-1, y+1, z+1).light)/4.0;
			lindA = (std::fmax(std::fmax(lindAf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindB = (std::fmax(std::fmax(lindBf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindC = (std::fmax(std::fmax(lindCf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindD = (std::fmax(std::fmax(lindDf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindE = (lindA+lindB+lindC+lindD)/4;
		}
		texX = (textureIndexes[cubeID][3] % (512/TEXSIZE))*TEXSIZE;
		texY = (textureIndexes[cubeID][3] / (512/TEXSIZE))*TEXSIZE;
		//t1
		renderData.push_back(Chunk::Vert(absX, absY+2, absZ+2, 3, texX+TEXSIZE  , texY          , lindD, lindD, lindD, 255));
		renderData.push_back(Chunk::Vert(absX, absY+2, absZ  , 3, texX          , texY          , lindA, lindA, lindA, 255));
		renderData.push_back(Chunk::Vert(absX, absY+1, absZ+1, 3, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t2
		renderData.push_back(Chunk::Vert(absX, absY+2, absZ  , 3, texX          , texY          , lindA, lindA, lindA, 255));
		renderData.push_back(Chunk::Vert(absX, absY  , absZ  , 3, texX	        , texY+TEXSIZE  , lindB, lindB, lindB, 255));
		renderData.push_back(Chunk::Vert(absX, absY+1, absZ+1, 3, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t3
		renderData.push_back(Chunk::Vert(absX, absY  , absZ  , 3, texX	        , texY+TEXSIZE  , lindB, lindB, lindB, 255));
		renderData.push_back(Chunk::Vert(absX, absY  , absZ+2, 3, texX+TEXSIZE  , texY+TEXSIZE  , lindC, lindC, lindC, 255));
		renderData.push_back(Chunk::Vert(absX, absY+1, absZ+1, 3, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t4
		renderData.push_back(Chunk::Vert(absX, absY  , absZ+2, 3, texX+TEXSIZE  , texY+TEXSIZE  , lindC, lindC, lindC, 255));
		renderData.push_back(Chunk::Vert(absX, absY+2, absZ+2, 3, texX+TEXSIZE  , texY          , lindD, lindD, lindD, 255));
		renderData.push_back(Chunk::Vert(absX, absY+1, absZ+1, 3, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
	}
	if(getCube(x, y-1, z).ID == 0) { // bottom face
		if (cubeID != 4) {
			unsigned char centerLight = getCube(x, y-1, z).light;
			lindAf = (centerLight + getCube(x+1, y-1, z).light +
					  getCube(x, y-1, z+1).light + getCube(x+1, y-1, z+1).light)/4.0;
			lindBf = (centerLight + getCube(x-1, y-1, z).light +
					  getCube(x, y-1, z+1).light + getCube(x-1, y-1, z+1).light)/4.0;
			lindCf = (centerLight + getCube(x-1, y-1, z).light +
					  getCube(x, y-1, z-1).light + getCube(x-1, y-1, z-1).light)/4.0;
			lindDf = (centerLight + getCube(x+1, y-1, z).light +
					  getCube(x, y-1, z-1).light + getCube(x+1, y-1, z-1).light)/4.0;
			lindA = (std::fmax(std::fmax(lindAf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindB = (std::fmax(std::fmax(lindBf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindC = (std::fmax(std::fmax(lindCf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindD = (std::fmax(std::fmax(lindDf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindE = (lindA+lindB+lindC+lindD)/4;
		}
		texX = (textureIndexes[cubeID][4] % (512/TEXSIZE))*TEXSIZE;
		texY = (textureIndexes[cubeID][4] / (512/TEXSIZE))*TEXSIZE;
		//t1
		renderData.push_back(Chunk::Vert(absX  , absY, absZ  , 4, texX+TEXSIZE  , texY+TEXSIZE  , lindC, lindC, lindC, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY, absZ  , 4, texX+TEXSIZE  , texY          , lindD, lindD, lindD, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY, absZ+1, 4, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t2
		renderData.push_back(Chunk::Vert(absX+2, absY, absZ  , 4, texX+TEXSIZE  , texY          , lindD, lindD, lindD, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY, absZ+2, 4, texX          , texY          , lindA, lindA, lindA, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY, absZ+1, 4, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t3
		renderData.push_back(Chunk::Vert(absX+2, absY, absZ+2, 4, texX          , texY          , lindA, lindA, lindA, 255));
		renderData.push_back(Chunk::Vert(absX  , absY, absZ+2, 4, texX	        , texY+TEXSIZE  , lindB, lindB, lindB, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY, absZ+1, 4, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t4
		renderData.push_back(Chunk::Vert(absX  , absY, absZ+2, 4, texX	        , texY+TEXSIZE  , lindB, lindB, lindB, 255));
		renderData.push_back(Chunk::Vert(absX  , absY, absZ  , 4, texX+TEXSIZE  , texY+TEXSIZE  , lindC, lindC, lindC, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY, absZ+1, 4, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
	}
	if(getCube(x, y+1, z).ID == 0) { // top face
		if (cubeID != 4) {
			unsigned char centerLight = getCube(x, y+1, z).light;
			lindAf = (centerLight + getCube(x-1, y+1, z).light +
					  getCube(x, y+1, z+1).light + getCube(x-1, y+1, z+1).light)/4.0;
			lindBf = (centerLight + getCube(x+1, y+1, z).light +
					  getCube(x, y+1, z+1).light + getCube(x+1, y+1, z+1).light)/4.0;
			lindCf = (centerLight + getCube(x+1, y+1, z).light +
					  getCube(x, y+1, z-1).light + getCube(x+1, y+1, z-1).light)/4.0;
			lindDf = (centerLight + getCube(x-1, y+1, z).light +
					  getCube(x, y+1, z-1).light + getCube(x-1, y+1, z-1).light)/4.0;
			lindA = (std::fmax(std::fmax(lindAf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindB = (std::fmax(std::fmax(lindBf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindC = (std::fmax(std::fmax(lindCf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindD = (std::fmax(std::fmax(lindDf, centerLight >> 2), MINLIGHT)/(MAXLIGHT))*255;
			lindE = (lindA+lindB+lindC+lindD)/4;
		}
		texX = (textureIndexes[cubeID][5] % (512/TEXSIZE))*TEXSIZE;
		texY = (textureIndexes[cubeID][5] / (512/TEXSIZE))*TEXSIZE;
		//t1
		renderData.push_back(Chunk::Vert(absX+2, absY+2, absZ  , 5, texX+TEXSIZE  , texY+TEXSIZE  , lindC, lindC, lindC, 255));
		renderData.push_back(Chunk::Vert(absX  , absY+2, absZ  , 5, texX+TEXSIZE  , texY          , lindD, lindD, lindD, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY+2, absZ+1, 5, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t2
		renderData.push_back(Chunk::Vert(absX  , absY+2, absZ  , 5, texX+TEXSIZE  , texY          , lindD, lindD, lindD, 255));
		renderData.push_back(Chunk::Vert(absX  , absY+2, absZ+2, 5, texX          , texY          , lindA, lindA, lindA, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY+2, absZ+1, 5, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t3
		renderData.push_back(Chunk::Vert(absX  , absY+2, absZ+2, 5, texX          , texY          , lindA, lindA, lindA, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY+2, absZ+2, 5, texX	      , texY+TEXSIZE  , lindB, lindB, lindB, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY+2, absZ+1, 5, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
		//t4
		renderData.push_back(Chunk::Vert(absX+2, absY+2, absZ+2, 5, texX	      , texY+TEXSIZE  , lindB, lindB, lindB, 255));
		renderData.push_back(Chunk::Vert(absX+2, absY+2, absZ  , 5, texX+TEXSIZE  , texY+TEXSIZE  , lindC, lindC, lindC, 255));
		renderData.push_back(Chunk::Vert(absX+1, absY+2, absZ+1, 5, texX+TEXSIZE/2, texY+TEXSIZE/2, lindE, lindE, lindE, 255));
	}
}
