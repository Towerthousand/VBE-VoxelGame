#include "Chunk.hpp"
#include "World.hpp"
#include "../DeferredContainer.hpp"
#include <cstring>
#include "../Manager.hpp"
#include "Sun.hpp"

#pragma GCC diagnostic ignored "-Wchar-subscripts"

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
vec3c Chunk::d[6] = {
	vec3c(-1,0,0),
	vec3c(1,0,0),
	vec3c(0,-1,0),
	vec3c(0,1,0),
	vec3c(0,0,-1),
	vec3c(0,0,1)
};


Chunk::Chunk(int x, unsigned int y, int z) :
	facesVisited(0),
	XPOS(x), YPOS(y), ZPOS(z),
	drawedByPlayer(false),
	needsMeshRebuild(true), hasVertices(false),
	visibilityGraph(0),
	boundingBox(vec3f(0),vec3f(0)),
	terrainModel(nullptr), boundingBoxModel(nullptr),
	world(nullptr), renderer(nullptr) {
	if(Game::i() != nullptr) {
		world = (World*)Game::i()->getObjectByName("world");
		renderer = (DeferredContainer*)Game::i()->getObjectByName("deferred");
	}
	memset(cubes,0,sizeof(cubes));
}

Chunk::~Chunk() {
	if(terrainModel != nullptr) delete terrainModel;
}

void Chunk::initStructures() {
	visibilityNodes.empty();
	for(unsigned int x = 0; x < CHUNKSIZE; x++)
		for(unsigned int y = 0; y < CHUNKSIZE; y++)
			for(unsigned int z = 0; z < CHUNKSIZE; z++)
				if(x == 0 || y == 0 || z == 0 || x == CHUNKSIZE-1 || y == CHUNKSIZE-1 || z == CHUNKSIZE-1)
					visibilityNodes.push_back(vec3c(x,y,z));
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
	drawedByPlayer = false;
}

void Chunk::draw() const {
	if(!hasVertices) return;
	if(renderer->getMode() == DeferredContainer::Deferred) {
		terrainModel->drawBatched(Programs.get("deferredChunk"));
		drawedByPlayer = true;
	}
	else if(renderer->getMode() == DeferredContainer::ShadowMap) {
		terrainModel->drawBatched(Programs.get("depthShader"));
	}
}

#define MAX_RAD 9
#define LIGHTSUM_SIZE (CHUNKSIZE+MAX_RAD+MAX_RAD)
static int lightSum[LIGHTSUM_SIZE][LIGHTSUM_SIZE][LIGHTSUM_SIZE];

void Chunk::calcLightSum() {
	for(int x = 0; x < LIGHTSUM_SIZE; x++)
		for(int y = 0; y < LIGHTSUM_SIZE; y++)
			for(int z = 0; z < LIGHTSUM_SIZE; z++)
				lightSum[x][y][z] = (getCube(x-MAX_RAD, y-MAX_RAD, z-MAX_RAD) == 0) ? 1 : 0;

	for(int x = 1; x < LIGHTSUM_SIZE; x++)
		for(int y = 0; y < LIGHTSUM_SIZE; y++)
			for(int z = 0; z < LIGHTSUM_SIZE; z++)
				lightSum[x][y][z] += lightSum[x-1][y][z];
	for(int x = 0; x < LIGHTSUM_SIZE; x++)
		for(int y = 1; y < LIGHTSUM_SIZE; y++)
			for(int z = 0; z < LIGHTSUM_SIZE; z++)
				lightSum[x][y][z] += lightSum[x][y-1][z];
	for(int x = 0; x < LIGHTSUM_SIZE; x++)
		for(int y = 0; y < LIGHTSUM_SIZE; y++)
			for(int z = 1; z < LIGHTSUM_SIZE; z++)
				lightSum[x][y][z] += lightSum[x][y][z-1];
}

int Chunk::sumRect(int x1, int y1, int z1, int x2, int y2, int z2) {
	using std::swap;
	if(x1 > x2) swap(x1, x2);
	if(y1 > y2) swap(y1, y2);
	if(z1 > z2) swap(z1, z2);

	x1--;
	y1--;
	z1--;

	x1 += MAX_RAD;
	y1 += MAX_RAD;
	z1 += MAX_RAD;
	x2 += MAX_RAD;
	y2 += MAX_RAD;
	z2 += MAX_RAD;

	return    lightSum[x2][y2][z2]
			- lightSum[x2][y2][z1] - lightSum[x2][y1][z2] - lightSum[x1][y2][z2]
			+ lightSum[x2][y1][z1] + lightSum[x1][y2][z1] + lightSum[x1][y1][z2]
			- lightSum[x1][y1][z1];
}

float Chunk::calcSubLight(int x, int y, int z, int dx, int dy, int dz, int d) {
	int x1 = dx ==  1 ? x : x-d;
	int x2 = dx == -1 ? x : x+d;
	int y1 = dy ==  1 ? y : y-d;
	int y2 = dy == -1 ? y : y+d;
	int z1 = dz ==  1 ? z : z-d;
	int z2 = dz == -1 ? z : z+d;

	return sumRect(x1, y1, z1, x2-1, y2-1, z2-1) / float(d*d*d*4);
}

unsigned char Chunk::calcLight(int x, int y, int z, int dx, int dy, int dz) {
	float light = 0;
	light += calcSubLight(x, y, z, dx, dy, dz, 1) * 0.5;
	light += calcSubLight(x, y, z, dx, dy, dz, 2) * 0.25;
	light += calcSubLight(x, y, z, dx, dy, dz, 4) * 0.15;
	light += calcSubLight(x, y, z, dx, dy, dz, 8) * 0.1;
	light = pow(light, 2);
	return light*255;
}

void Chunk::rebuildMesh() {
	if(!needsMeshRebuild) return;
	needsMeshRebuild = false;
	if(terrainModel == nullptr) initMesh();
	std::vector<Chunk::Vert> renderData;
	boundingBox = AABB();

	calcLightSum();

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
	terrainModel->setVertexData(&renderData[0], renderData.size());
	hasVertices = (renderData.size() != 0);
	rebuildVisibilityGraph();
}

bool Chunk::visibilityTest(Chunk::Face exit) const {
	if(visibilityGraph.all()) return true;
	for(int i = 0; i < 6; ++i)
		if(facesVisited.test(i) && visibilityGraph.test(getVisibilityIndex(i, exit)))
			return true;
	return false;
}

void Chunk::initMesh() {
	std::vector<Vertex::Element> elements = {
		Vertex::Element(Vertex::Attribute::Position, Vertex::Element::UnsignedByte, 3, Vertex::Element::ConvertToFloat),
		Vertex::Element(Vertex::Attribute::Normal, Vertex::Element::UnsignedByte, 1),
		Vertex::Element(Vertex::Attribute::TexCoord, Vertex::Element::UnsignedShort, 2, Vertex::Element::ConvertToFloat),
		Vertex::Element(Vertex::Attribute::get("a_light"), Vertex::Element::UnsignedByte, 1, Vertex::Element::ConvertToFloatNormalized)
	};
	terrainModel = new MeshBatched(Vertex::Format(elements));
	boundingBoxModel = Meshes.get("1x1Cube");
}

void Chunk::rebuildVisibilityGraph() {
	visibilityGraph.reset();
	bool visited[CHUNKSIZE][CHUNKSIZE][CHUNKSIZE];
	memset(&visited,0, sizeof(bool)*CHUNKSIZE*CHUNKSIZE*CHUNKSIZE);
	for(unsigned int i = 0; i < visibilityNodes.size(); ++i) {
		vec3c& src = visibilityNodes[i];
		if(visited[src.x][src.y][src.z] || cubes[src.x][src.y][src.z] != 0) continue;
		std::bitset<6> faces(0); //faces the current bfs has touched
		std::queue<vec3c> q;
		q.push(src);
		visited[src.x][src.y][src.z] = true; //visited by any bfs
		while(!q.empty()) {
			vec3c c = q.front(); q.pop(); //current
			if(c.x == 0) faces.set(MINX);
			else if(c.x == CHUNKSIZE-1) faces.set(MAXX);
			if(c.y == 0) faces.set(MINY);
			else if(c.y == CHUNKSIZE-1) faces.set(MAXY);
			if(c.z == 0) faces.set(MINZ);
			else if(c.z == CHUNKSIZE-1) faces.set(MAXZ);
			for(int j = 0; j < 6; ++j) {
				vec3c n = c + d[j]; //neighbor
				//cull out-of-chunk nodes
				if(n.x < 0 || n.y < 0 || n.z < 0 || n.x == CHUNKSIZE || n.y == CHUNKSIZE || n.z == CHUNKSIZE) continue;

				//don't visit non-air nodes and omit already visited nodes
				if(visited[n.x][n.y][n.z] || cubes[n.x][n.y][n.z] != 0) continue;

				visited[n.x][n.y][n.z] = true;
				q.push(n);
			}
			if(faces.all()) { visibilityGraph.set(); return; }
		}
		for(int i = 0; i < 6; ++ i)
			for(int j = i+1; j < 6; ++j) {
				if(!(faces.test(i) && faces.test(j))) continue;
				visibilityGraph.set(getVisibilityIndex(i,j));
				visibilityGraph.set(getVisibilityIndex(j,i));
			}

		if(visibilityGraph.all()) return;
	}
}

void Chunk::pushCubeToArray(short x, short y, short z, std::vector<Chunk::Vert> &renderData) { //I DON'T KNOW HOW TO MAKE THIS COMPACT
	short texY, texX;
	unsigned int cubeID = cubes[x][y][z];
	std::vector<Vert> v(6);

	if(getCube(x, y, z+1) == 0) { // front face
		texX = (textureIndexes[cubeID][0] % (512/TEXSIZE))*TEXSIZE; // TEXSIZE/2 = number of textures/row
		texY = (textureIndexes[cubeID][0] / (512/TEXSIZE))*TEXSIZE; // TEXSIZE/2 = number of textures/row

		v[0] = Chunk::Vert(x+1, y+1, z+1, 0, texX        , texY          , calcLight(x+1, y+1, z+1, 0, 0, 1));
		v[1] = Chunk::Vert(x  , y+1, z+1, 0, texX+TEXSIZE, texY          , calcLight(x  , y+1, z+1, 0, 0, 1));
		v[2] = Chunk::Vert(x+1, y  , z+1, 0, texX        , texY+TEXSIZE  , calcLight(x+1, y  , z+1, 0, 0, 1));

		v[3] = Chunk::Vert(x  , y  , z+1, 0, texX+TEXSIZE, texY+TEXSIZE  , calcLight(x  , y  , z+1, 0, 0, 1));
		v[4] = Chunk::Vert(x+1, y  , z+1, 0, texX        , texY+TEXSIZE  , calcLight(x+1, y  , z+1, 0, 0, 1));
		v[5] = Chunk::Vert(x  , y+1, z+1, 0, texX+TEXSIZE, texY          , calcLight(x  , y+1, z+1, 0, 0, 1));

		if((v[1].l + v[2].l) < (v[0].l + v[3].l)) {
			v[2] = v[3];
			v[5] = v[0];
		}

		renderData.insert(renderData.end(), v.begin(), v.end());
	}
	if(getCube(x, y, z-1) == 0) { // back face
		texX = (textureIndexes[cubeID][1] % (512/TEXSIZE))*TEXSIZE;
		texY = (textureIndexes[cubeID][1] / (512/TEXSIZE))*TEXSIZE;

		v[0] = Chunk::Vert(x+1, y  , z, 1, texX          , texY+TEXSIZE  , calcLight(x+1, y  , z, 0, 0, -1));
		v[1] = Chunk::Vert(x  , y+1, z, 1, texX+TEXSIZE  , texY          , calcLight(x  , y+1, z, 0, 0, -1));
		v[2] = Chunk::Vert(x+1, y+1, z, 1, texX          , texY          , calcLight(x+1, y+1, z, 0, 0, -1));

		v[3] = Chunk::Vert(x  , y  , z, 1, texX+TEXSIZE  , texY+TEXSIZE  , calcLight(x  , y  , z, 0, 0, -1));
		v[4] = Chunk::Vert(x  , y+1, z, 1, texX+TEXSIZE  , texY          , calcLight(x  , y+1, z, 0, 0, -1));
		v[5] = Chunk::Vert(x+1, y  , z, 1, texX          , texY+TEXSIZE  , calcLight(x+1, y  , z, 0, 0, -1));

		if((v[0].l + v[1].l) < (v[2].l + v[3].l)) {
			v[0] = v[3];
			v[4] = v[2];
		}

		renderData.insert(renderData.end(), v.begin(), v.end());
	}
	if(getCube(x+1, y, z) == 0) { // left face
		texX = (textureIndexes[cubeID][2] % (512/TEXSIZE))*TEXSIZE;
		texY = (textureIndexes[cubeID][2] / (512/TEXSIZE))*TEXSIZE;

		v[0] = Chunk::Vert(x+1, y  , z+1, 2, texX        , texY+TEXSIZE  , calcLight(x+1, y  , z+1, 1, 0, 0));
		v[1] = Chunk::Vert(x+1, y  , z  , 2, texX+TEXSIZE, texY+TEXSIZE  , calcLight(x+1, y  , z  , 1, 0, 0));
		v[2] = Chunk::Vert(x+1, y+1, z+1, 2, texX        , texY          , calcLight(x+1, y+1, z+1, 1, 0, 0));

		v[3] = Chunk::Vert(x+1, y  , z  , 2, texX+TEXSIZE, texY+TEXSIZE  , calcLight(x+1, y  , z  , 1, 0, 0));
		v[4] = Chunk::Vert(x+1, y+1, z  , 2, texX+TEXSIZE, texY          , calcLight(x+1, y+1, z  , 1, 0, 0));
		v[5] = Chunk::Vert(x+1, y+1, z+1, 2, texX        , texY          , calcLight(x+1, y+1, z+1, 1, 0, 0));

		if((v[1].l + v[2].l) < (v[0].l + v[4].l)) {
			v[1] = v[4];
			v[5] = v[0];
		}

		renderData.insert(renderData.end(), v.begin(), v.end());
	}
	if(getCube(x-1, y, z) == 0) { // right face
		texX = (textureIndexes[cubeID][3] % (512/TEXSIZE))*TEXSIZE;
		texY = (textureIndexes[cubeID][3] / (512/TEXSIZE))*TEXSIZE;

		v[0] = Chunk::Vert(x  , y  , z+1, 3, texX		 , texY+TEXSIZE  , calcLight(x  , y  , z+1, -1, 0, 0));
		v[1] = Chunk::Vert(x  , y+1, z+1, 3, texX        , texY          , calcLight(x  , y+1, z+1, -1, 0, 0));
		v[2] = Chunk::Vert(x  , y  , z  , 3, texX+TEXSIZE, texY+TEXSIZE  , calcLight(x  , y  , z  , -1, 0, 0));

		v[3] = Chunk::Vert(x  , y+1, z+1, 3, texX        , texY          , calcLight(x  , y+1, z+1, -1, 0, 0));
		v[4] = Chunk::Vert(x  , y+1, z  , 3, texX+TEXSIZE, texY          , calcLight(x  , y+1, z  , -1, 0, 0));
		v[5] = Chunk::Vert(x  , y  , z  , 3, texX+TEXSIZE, texY+TEXSIZE  , calcLight(x  , y  , z  , -1, 0, 0));

		if((v[1].l + v[2].l) < (v[0].l + v[4].l)) {
			v[1] = v[4];
			v[5] = v[0];
		}

		renderData.insert(renderData.end(), v.begin(), v.end());
	}
	if(getCube(x, y-1, z) == 0) { // bottom face
		texX = (textureIndexes[cubeID][4] % (512/TEXSIZE))*TEXSIZE;
		texY = (textureIndexes[cubeID][4] / (512/TEXSIZE))*TEXSIZE;

		v[0] = Chunk::Vert(x+1, y, z  , 4, texX+TEXSIZE  , texY          , calcLight(x+1, y, z  , 0, -1, 0));
		v[1] = Chunk::Vert(x  , y, z+1, 4, texX          , texY+TEXSIZE  , calcLight(x  , y, z+1, 0, -1, 0));
		v[2] = Chunk::Vert(x  , y, z  , 4, texX          , texY          , calcLight(x  , y, z  , 0, -1, 0));

		v[3] = Chunk::Vert(x+1, y, z  , 4, texX+TEXSIZE  , texY          , calcLight(x+1, y, z  , 0, -1, 0));
		v[4] = Chunk::Vert(x+1, y, z+1, 4, texX+TEXSIZE  , texY+TEXSIZE  , calcLight(x+1, y, z+1, 0, -1, 0));
		v[5] = Chunk::Vert(x  , y, z+1, 4, texX          , texY+TEXSIZE  , calcLight(x  , y, z+1, 0, -1, 0));

		if((v[0].l + v[1].l) < (v[2].l + v[4].l)) {
			v[1] = v[4];
			v[3] = v[2];
		}

		renderData.insert(renderData.end(), v.begin(), v.end());
	}
	if(getCube(x, y+1, z) == 0) { // top face
		texX = (textureIndexes[cubeID][5] % (512/TEXSIZE))*TEXSIZE;
		texY = (textureIndexes[cubeID][5] / (512/TEXSIZE))*TEXSIZE;

		v[0] = Chunk::Vert(x+1, y+1, z  , 5, texX+TEXSIZE, texY          , calcLight(x+1, y+1, z  , 0, 1, 0));
		v[1] = Chunk::Vert(x  , y+1, z  , 5, texX        , texY          , calcLight(x  , y+1, z  , 0, 1, 0));
		v[2] = Chunk::Vert(x  , y+1, z+1, 5, texX        , texY+TEXSIZE  , calcLight(x  , y+1, z+1, 0, 1, 0));

		v[3] = Chunk::Vert(x+1, y+1, z  , 5, texX+TEXSIZE, texY          , calcLight(x+1, y+1, z  , 0, 1, 0));
		v[4] = Chunk::Vert(x  , y+1, z+1, 5, texX        , texY+TEXSIZE  , calcLight(x  , y+1, z+1, 0, 1, 0));
		v[5] = Chunk::Vert(x+1, y+1, z+1, 5, texX+TEXSIZE, texY+TEXSIZE  , calcLight(x+1, y+1, z+1, 0, 1, 0));

		if((v[0].l + v[2].l) < (v[1].l + v[5].l)) {
			v[2] = v[5];
			v[3] = v[1];
		}

		renderData.insert(renderData.end(), v.begin(), v.end());
	}
}
