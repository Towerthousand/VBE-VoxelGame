#include "Chunk.hpp"
#include "World.hpp"
#include "../DeferredContainer.hpp"

const int Chunk::textureIndexes[9][6] = { //order is front, back, left, right, bottom, top
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

std::vector<vec3c> Chunk::visibilityNodes;
bool Chunk::visited[CHUNKSIZE][CHUNKSIZE][CHUNKSIZE];
vec3c Chunk::d[6] = {
	vec3c(-1,0,0),
	vec3c(1,0,0),
	vec3c(0,-1,0),
	vec3c(0,1,0),
	vec3c(0,0,-1),
	vec3c(0,0,1)
};


Chunk::Chunk(int x, unsigned int y, int z) :
	XPOS(x), YPOS(y), ZPOS(z),
	needsMeshRebuild(true), hasVertices(false),
	visibilityFlags(0x0000), modelMatrix(mat4f(1.0f)),
	boundingBox(vec3f(0),vec3f(0)),
	world(nullptr), renderer(nullptr) {
	if(Game::i() != nullptr) {
		world = (World*)Game::i()->getObjectByName("world");
		renderer = (DeferredContainer*)Game::i()->getObjectByName("deferred");
	}
	modelMatrix = glm::translate(modelMatrix, vec3f(XPOS*CHUNKSIZE, YPOS*CHUNKSIZE, ZPOS*CHUNKSIZE));
	memset(cubes,0,sizeof(cubes));
}

Chunk::~Chunk() {
	delete terrainModel.mesh;
}

void Chunk::initStructures() {
	visibilityNodes.empty();
	for(unsigned int x = 0; x < CHUNKSIZE; x++)
		for(unsigned int y = 0; y < CHUNKSIZE; y++)
			for(unsigned int z = 0; z < CHUNKSIZE; z++)
				if(x == 0 || y == 0 || z == 0 || x == CHUNKSIZE-1 || y == CHUNKSIZE-1 || z == CHUNKSIZE-1)
					visibilityNodes.push_back(vec3c(x,y,z));
}

unsigned short Chunk::getVisibilityFlagsForFaces(unsigned char faces) {
	unsigned short ret = 0x0000;
	if((faces & MINX) != 0x00 && (faces & MAXX) != 0x00) ret |= MINX_MAXX;
	if((faces & MINX) != 0x00 && (faces & MINY) != 0x00) ret |= MINX_MINY;
	if((faces & MINX) != 0x00 && (faces & MAXY) != 0x00) ret |= MINX_MAXY;
	if((faces & MINX) != 0x00 && (faces & MINZ) != 0x00) ret |= MINX_MINZ;
	if((faces & MINX) != 0x00 && (faces & MAXZ) != 0x00) ret |= MINX_MAXX;
	if((faces & MINY) != 0x00 && (faces & MAXX) != 0x00) ret |= MINY_MAXX;
	if((faces & MINY) != 0x00 && (faces & MAXY) != 0x00) ret |= MINY_MAXY;
	if((faces & MINY) != 0x00 && (faces & MINZ) != 0x00) ret |= MINY_MINZ;
	if((faces & MINY) != 0x00 && (faces & MAXZ) != 0x00) ret |= MINY_MAXZ;
	if((faces & MINZ) != 0x00 && (faces & MAXX) != 0x00) ret |= MINZ_MAXX;
	if((faces & MINZ) != 0x00 && (faces & MAXY) != 0x00) ret |= MINZ_MAXY;
	if((faces & MINZ) != 0x00 && (faces & MAXZ) != 0x00) ret |= MINZ_MAXZ;
	if((faces & MAXX) != 0x00 && (faces & MAXY) != 0x00) ret |= MAXX_MAXY;
	if((faces & MAXX) != 0x00 && (faces & MAXZ) != 0x00) ret |= MAXX_MAXZ;
	if((faces & MAXY) != 0x00 && (faces & MAXZ) != 0x00) ret |= MAXY_MAXZ;
	return ret;
}

Chunk::Face Chunk::getOppositeFace(Chunk::Face f) {
	switch(f) {
		case MINX: return MAXX;
		case MINY: return MAXY;
		case MINZ: return MAXZ;
		case MAXX: return MINX;
		case MAXY: return MINY;
		case MAXZ: return MINZ;
		case ALL_FACES: return ALL_FACES;
	}
	return ALL_FACES;
}

void Chunk::update(float deltaTime) {
	(void) deltaTime;
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

void Chunk::rebuildMesh() {
	if(!needsMeshRebuild) return;
	needsMeshRebuild = false;
	if(terrainModel.mesh == nullptr) initMesh();
	std::vector<Chunk::Vert> renderData;
	boundingBox = AABB();
	for(int z = 0; z < CHUNKSIZE; ++z)
		for(int y = 0; y < CHUNKSIZE; ++y)
			for(int x = 0; x < CHUNKSIZE; ++x)
				if (cubes[x][y][z] != 0) { // only draw if it's not air
					unsigned int oldSize = renderData.size();
					pushCubeToArray(x, y, z, renderData);
					if(renderData.size() > oldSize){
						boundingBox.extend(vec3f(x, y, z));
						boundingBox.extend(vec3f(x+1, y+1, z+1));
					}
				}
	terrainModel.mesh->setVertexData(&renderData[0], renderData.size());
	hasVertices = (renderData.size() != 0);
	rebuildVisibilityGraph();
}

vec3i Chunk::getAbsolutePos() const {
	return vec3i(XPOS*CHUNKSIZE, YPOS*CHUNKSIZE, ZPOS*CHUNKSIZE);
}

AABB Chunk::getWorldSpaceBoundingBox() const {
	return AABB(vec3f(modelMatrix*vec4f(boundingBox.getMin(),1)), vec3f(modelMatrix*vec4f(boundingBox.getMax(),1)));
}

bool Chunk::visibilityTest(Chunk::Face enter, Chunk::Face exit) const {
	return ((getVisibilityFlagsForFaces(enter|exit) & visibilityFlags) != 0x0000);
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

void Chunk::rebuildVisibilityGraph() {
	memset(&visited,0, sizeof(bool)*CHUNKSIZE*CHUNKSIZE*CHUNKSIZE);
	visibilityFlags = 0x0000; //the flags
	std::queue<vec3c> q;
	unsigned char faces;
	for(unsigned int i = 0; i < visibilityNodes.size(); ++i) {
		vec3c& src = visibilityNodes[i];
		if(visited[src.x][src.y][src.z] || cubes[src.x][src.y][src.z] != 0) continue;
		faces = 0x00; //faces the current bfs has touched
		q.empty();
		q.push(src);
		visited[src.x][src.y][src.z] = true; //visited by any bfs?
		while(!q.empty()) {
			vec3c c = q.front(); q.pop();
			if(c.x == 0) faces |= MINX;
			else if(c.x == CHUNKSIZE-1) faces |= MAXX;
			if(c.y == 0) faces |= MINY;
			else if(c.y == CHUNKSIZE-1) faces |= MAXY;
			if(c.z == 0) faces |= MINZ;
			else if(c.z == CHUNKSIZE-1) faces |= MAXZ;
			for(int j = 0; j < 6; ++j) {
				vec3c neighbor = c + d[j];
				//cull out-of-chunk nodes
				if(neighbor.x < 0 || neighbor.y < 0 || neighbor.z < 0 ||
				   neighbor.x == CHUNKSIZE || neighbor.y == CHUNKSIZE || neighbor.z == CHUNKSIZE) continue;

				//don't visit non-air nodes and omit already visited nodes
				if(visited[neighbor.x][neighbor.y][neighbor.z] || cubes[neighbor.x][neighbor.y][neighbor.z] != 0) continue;

				visited[neighbor.x][neighbor.y][neighbor.z] = true;
				q.push(neighbor);
			}
			if(faces == ALL_FACES) { visibilityFlags = ALL_FLAGS; return; }
		}

		visibilityFlags |= getVisibilityFlagsForFaces(faces);

		if(visibilityFlags == ALL_FLAGS) return;
	}
}

unsigned int Chunk::getCube(int x, int y, int z) const { //in local space
	if(x >= 0 && x < CHUNKSIZE && y >= 0 && y < CHUNKSIZE && z >= 0 && z < CHUNKSIZE)
		return cubes[x][y][z];
	return world->getCube(x+(XPOS*CHUNKSIZE), y+(YPOS*CHUNKSIZE), z+(ZPOS*CHUNKSIZE)); //in another chunk
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
