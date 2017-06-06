#include "Chunk.hpp"
#include "World.hpp"
#include "../DeferredContainer.hpp"
#include <cstring>
#include "../Manager.hpp"
#include "Sun.hpp"
#include "Cube.hpp"

#pragma GCC diagnostic ignored "-Wchar-subscripts"

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
    XPOS(x), YPOS(y), ZPOS(z) {
    if(Game::i() != nullptr) {
        world = (World*)Game::i()->getObjectByName("world");
        renderer = (DeferredContainer*)Game::i()->getObjectByName("deferred");
    }
    memset(cubes,0,sizeof(cubes));
}

Chunk::~Chunk() {
    if(terrainModel != nullptr) delete terrainModel;
    if(transModel != nullptr) delete transModel;
}

void Chunk::initStructures() {
    visibilityNodes.clear();
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
    if(renderer->getMode() == DeferredContainer::Deferred) {
        terrainModel->drawBatched(Programs.get("deferredChunk"));
        drawedByPlayer = true;
    }
    else if(renderer->getMode() == DeferredContainer::ShadowMap) {
        terrainModel->drawBatched(Programs.get("depthShader"));
    }
    else if(renderer->getMode() == DeferredContainer::TransShadowMap) {
        transModel->drawBatched(Programs.get("depthShader"));
    }
    else if(renderer->getMode() == DeferredContainer::Forward) {
        transModel->drawBatched(Programs.get("forwardChunk"));
    }
}

#define LIGHTSUM_SIZE (CHUNKSIZE+AO_MAX_RAD+AO_MAX_RAD)
static int lightSum[LIGHTSUM_SIZE][LIGHTSUM_SIZE][LIGHTSUM_SIZE];

void Chunk::calcLightSum() {
    for(int x = 0; x < LIGHTSUM_SIZE; x++)
        for(int y = 0; y < LIGHTSUM_SIZE; y++)
            for(int z = 0; z < LIGHTSUM_SIZE; z++)
                lightSum[x][y][z] = (getCube(x-AO_MAX_RAD, y-AO_MAX_RAD, z-AO_MAX_RAD) == 0) ? 1 : 0;
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

int Chunk::sumRect(int x1, int y1, int z1, int x2, int y2, int z2) const {
    using std::swap;
    if(x1 > x2) swap(x1, x2);
    if(y1 > y2) swap(y1, y2);
    if(z1 > z2) swap(z1, z2);

    x1--;
    y1--;
    z1--;

    x1 += AO_MAX_RAD;
    y1 += AO_MAX_RAD;
    z1 += AO_MAX_RAD;
    x2 += AO_MAX_RAD;
    y2 += AO_MAX_RAD;
    z2 += AO_MAX_RAD;

    return    lightSum[x2][y2][z2]
            - lightSum[x2][y2][z1] - lightSum[x2][y1][z2] - lightSum[x1][y2][z2]
            + lightSum[x2][y1][z1] + lightSum[x1][y2][z1] + lightSum[x1][y1][z2]
            - lightSum[x1][y1][z1];
}

float Chunk::calcSubLight(int x, int y, int z, int dx, int dy, int dz, int d) const {
    int x1 = dx ==  1 ? x : x-d;
    int x2 = dx == -1 ? x : x+d;
    int y1 = dy ==  1 ? y : y-d;
    int y2 = dy == -1 ? y : y+d;
    int z1 = dz ==  1 ? z : z-d;
    int z2 = dz == -1 ? z : z+d;

    return sumRect(x1, y1, z1, x2-1, y2-1, z2-1) / float(d*d*d*4);
}

unsigned char Chunk::calcLight(int x, int y, int z, int dx, int dy, int dz) const {
    float light = 0;
    light += calcSubLight(x, y, z, dx, dy, dz, 1) * 0.5;
    light += calcSubLight(x, y, z, dx, dy, dz, 2) * 0.25;
    light += calcSubLight(x, y, z, dx, dy, dz, 4) * 0.15;
    light += calcSubLight(x, y, z, dx, dy, dz, 8) * 0.1;
    light = pow(light, 2);
    return light*255;
}

bool Chunk::isSurrounded() const {
    vec3i p = getAbsolutePos();
    for(int x = -1; x <= 1; ++x)
        for(int y = -1; y <= 1; ++y)
            if(world->outOfBounds(p + vec3i(CHUNKSIZE*x, 0, CHUNKSIZE*y))) return false;
    return true;
}

void Chunk::rebuildMesh() {
    if(!needsMeshRebuild) return;
    if(!isSurrounded()) return;
    needsMeshRebuild = false;
    if(terrainModel == nullptr) initMesh();
    std::vector<Cube::Vert> renderData;
    renderData.reserve(terrainModel->getVertexCount());
    std::vector<Cube::Vert> transRenderData;
    transRenderData.reserve(transModel->getVertexCount());
    boundingBox = AABB();
    calcLightSum();
    for(int z = 0; z < CHUNKSIZE; ++z)
        for(int y = 0; y < CHUNKSIZE; ++y)
            for(int x = 0; x < CHUNKSIZE; ++x) {
                Cube::Type c = (Cube::Type) cubes[x][y][z];
                if(c != Cube::AIR) {
                    if(Cube::getFlag(c, Cube::TRANSPARENT)) {
                        unsigned int oldSize = transRenderData.size();
                        pushCubeToArray(x, y, z, c, this, transRenderData);
                        if(transRenderData.size() > oldSize){
                            boundingBox.extend(vec3f(x, y, z));
                            boundingBox.extend(vec3f(x+1, y+1, z+1));
                        }
                    }
                    else {
                        unsigned int oldSize = renderData.size();
                        pushCubeToArray(x, y, z, c, this, renderData);
                        if(renderData.size() > oldSize){
                            boundingBox.extend(vec3f(x, y, z));
                            boundingBox.extend(vec3f(x+1, y+1, z+1));
                        }
                    }
                }
            }
    terrainModel->setVertexData(&renderData[0], renderData.size());
    transModel->setVertexData(&transRenderData[0], transRenderData.size());
    hasVertices = (!renderData.empty() || !transRenderData.empty());
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
    std::vector<Vertex::Attribute> elements = {
        Vertex::Attribute("a_position", Vertex::Attribute::UnsignedByte, 3, Vertex::Attribute::ConvertToFloat),
        Vertex::Attribute("a_normal", Vertex::Attribute::UnsignedByte, 1),
        Vertex::Attribute("a_texCoord", Vertex::Attribute::UnsignedShort, 2, Vertex::Attribute::ConvertToFloat),
        Vertex::Attribute("a_light", Vertex::Attribute::UnsignedByte, 1, Vertex::Attribute::ConvertToFloatNormalized)
    };
    terrainModel = new MeshBatched(Vertex::Format(elements));
    transModel = new MeshBatched(Vertex::Format(elements));
    boundingBoxModel = &Meshes.get("1x1Cube");
}

void Chunk::rebuildVisibilityGraph() {
    visibilityGraph.reset();
    bool visited[CHUNKSIZE][CHUNKSIZE][CHUNKSIZE];
    memset(&visited, 0, sizeof(bool)*CHUNKSIZE*CHUNKSIZE*CHUNKSIZE);
    for(unsigned int i = 0; i < visibilityNodes.size(); ++i) {
        vec3c& src = visibilityNodes[i];
        if(visited[src.x][src.y][src.z] || !Cube::getFlag((Cube::Type)cubes[src.x][src.y][src.z], Cube::TRANSPARENT)) continue;
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

                //don't visit already visited nodes
                if(visited[n.x][n.y][n.z])
                    continue;
                //don't visit non-transparent nodes
                if(!Cube::getFlag((Cube::Type)cubes[n.x][n.y][n.z], Cube::TRANSPARENT))
                    continue;

                visited[n.x][n.y][n.z] = true;
                q.push(n);
            }
            // Early exit if we have reached all faces
            if(faces.all()) {
                visibilityGraph.set();
                return;
            }
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
