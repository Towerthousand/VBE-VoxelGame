#include "World.hpp"
#include "Column.hpp"
#include "Chunk.hpp"
#include "../DeferredContainer.hpp"
#include "../Debugger.hpp"
#include "../Manager.hpp"
#include "Sun.hpp"
#include <cstring>
#include "DeferredCubeLight.hpp"

World::World() : generator(rand()) {
    renderer = (DeferredContainer*)getGame()->getObjectByName("deferred");
    setName("world");
    for(int x = 0; x < WORLDSIZE; ++x)
        for(int z = 0; z < WORLDSIZE; ++z)
            columns[x][z] = nullptr;
    Sun* sun = new Sun();
    sun->addTo(this);
    Chunk::initStructures();
}

World::~World() {
}

void World::setCube(int x, int y, int z, unsigned int cube) {
    setCubeData(x, y, z, cube);
    for(int a = -AO_MAX_RAD; a < AO_MAX_RAD+1; a += AO_MAX_RAD)
        for(int b = -AO_MAX_RAD; b < AO_MAX_RAD+1; b += AO_MAX_RAD)
            for(int c = -AO_MAX_RAD; c < AO_MAX_RAD+1; c += AO_MAX_RAD)
                recalc(x+a,y+b,z+c);
    std::vector<DeferredCubeLight*> lights;
    getAllObjectsOfType(lights);
    for(DeferredCubeLight* l : lights)
        l->calcLight(x,y,z);
}

void World::setCubeRange(int x, int y, int z, unsigned int sx, unsigned int sy, unsigned int sz, unsigned int cube) {
    int x2 = x+sx;
    int y2 = y+sy;
    int z2 = z+sz;
    for(int i = x; i < x2; ++i)
        for(int j = y; j < y2; ++j)
            for(int k = z; k < z2; ++k)
                setCubeData(i,j,k, cube);
    std::vector<DeferredCubeLight*> lights;
    getAllObjectsOfType(lights);
    for(DeferredCubeLight* l : lights) {
        vec3f p = glm::floor(l->getPosition());
        int x3 = (p.x >= x) ? ((p.x <= x2) ? p.x : x2) : x;
        int y3 = (p.y >= y) ? ((p.y <= y2) ? p.y : y2) : y;
        int z3 = (p.z >= z) ? ((p.z <= z2) ? p.z : z2) : z;
        l->calcLight(x3,y3,z3);
    }
    for(int i = ((x-AO_MAX_RAD) & (~CHUNKSIZE_MASK)); i < ((x2+AO_MAX_RAD+CHUNKSIZE) & (~CHUNKSIZE_MASK)); i += CHUNKSIZE)
        for(int j = ((y-AO_MAX_RAD) & (~CHUNKSIZE_MASK)); j < ((y2+AO_MAX_RAD+CHUNKSIZE) & (~CHUNKSIZE_MASK)); j += CHUNKSIZE)
            for(int k = ((z-AO_MAX_RAD) & (~CHUNKSIZE_MASK)); k < ((z2+AO_MAX_RAD+CHUNKSIZE) & (~CHUNKSIZE_MASK)); k += CHUNKSIZE)
                recalc(i,j,k);
}

void World::update(float deltaTime) {
    (void) deltaTime;
}

void World::fixedUpdate(float deltaTime) {
    Debugger::pushMark("World Fixed Update", "Time taken to update all blocks and insert new chunks");
    generator.discardTasks();
    Column* newCol = nullptr;
    while((newCol = generator.pullDone()) != nullptr) {
        if(getColumnCC(newCol->getX(), 0, newCol->getZ()) != nullptr) delete newCol;
        else columns[newCol->getX()&WORLDSIZE_MASK][newCol->getZ()&WORLDSIZE_MASK] = newCol;
    }
    Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");
    vec2f playerChunkPos = vec2f(vec2i(cam->getWorldPos().x,cam->getWorldPos().z) >> CHUNKSIZE_POW2);
    std::vector<std::pair<float,std::pair<int,int> > > tasks;
    highestChunkY = 0;
    for(int x = -WORLDSIZE/2; x < WORLDSIZE/2; ++x)
        for(int z = -WORLDSIZE/2; z < WORLDSIZE/2; ++z) {
            vec2f colPos = playerChunkPos + vec2f(x,z);
            Column* actual = getColumnCC(colPos.x,0,colPos.y);
            if(actual == nullptr && !generator.currentlyWorking(vec2i(colPos))) {
                Column*& realpos = columns[int(colPos.x)&WORLDSIZE_MASK][int(colPos.y)&WORLDSIZE_MASK];
                if(realpos != nullptr) delete realpos;
                tasks.push_back(std::pair<float,std::pair<int,int> >(glm::length(playerChunkPos-colPos),std::pair<int,int>(colPos.x,colPos.y)));
                realpos = nullptr;
                continue;
            }
            if(actual == nullptr) continue;
            highestChunkY = std::max(actual->getChunkCount(), highestChunkY);
            for(unsigned int y = 0; y < actual->getChunkCount(); ++y){
                Chunk* c = actual->getChunkCC(y);
                if(c == nullptr) continue;
                c->update(deltaTime);
            }
        }
    std::sort(tasks.begin(),tasks.end());
    for(unsigned int i = 0; i < tasks.size(); ++i) generator.enqueueTask(vec2i(tasks[i].second.first,tasks[i].second.second));
    Debugger::popMark();
}

void World::draw() const {
    switch(renderer->getMode()) {
        case DeferredContainer::Deferred:
            Debugger::pushMark("Player Cam World Draw", "Time spent drawing chunks from the player camera");
            drawPlayerCam((Camera*)getGame()->getObjectByName("playerCam"));
            Debugger::popMark(); //world draw
            break;
        case DeferredContainer::ShadowMap: {
            Debugger::pushMark("Sun Cam World Draw", "Time spent drawing chunks from the sun camera");
            Sun* sun = (Sun*)getGame()->getObjectByName("sun");
            sun->updateCameras();
            //drawSunCam(sun->getGlobalCam());
            Debugger::popMark(); //world draw
            break;
        }
        default: break;
    }
}

static Chunk::Face faces[6] = {
    Chunk::MINX, Chunk::MAXX,
    Chunk::MINY, Chunk::MAXY,
    Chunk::MINZ, Chunk::MAXZ
};

static vec3i offsets[6] = {
    vec3i(-1,0,0), vec3i(1,0,0),
    vec3i(0,-1,0), vec3i(0,1,0),
    vec3i(0,0,-1), vec3i(0,0,1)
};

struct Job {
        vec3i pos;
        int distance;
};

bool visited[WORLDSIZE][WORLDSIZE][WORLDSIZE];

void World::drawPlayerCam(const Camera* cam) const{
    VBE_ASSERT(renderer->getMode() == DeferredContainer::Deferred, "Wat");
    std::list<Chunk*> chunksToDraw; //push all the chunks that must be drawn here
    vec3i initialChunkPos(vec3i(glm::floor(cam->getWorldPos())) >> CHUNKSIZE_POW2);
    std::queue<Job> q; //bfs queue, each node is (entry face, chunkPos, distance to source), in chunk coords
    for(int i = 0; i < 6; ++i) {
        Chunk* aux;
        //push chunk with current face
        q.push({initialChunkPos, 0});
        aux = getChunkCC(initialChunkPos); //used later inside neighbor loop
        if(aux != nullptr) aux->facesVisited.set(i);
    }
    Sphere colliderSphere(vec3f(0.0f), (CHUNKSIZE >> 1)*1.74f);
    vec3i colliderOffset = vec3i(CHUNKSIZE >> 1);
    memset(visited, 0, sizeof(visited));
    Debugger::pushMark("Deferred BFS", "CPU BFS for the deferred pass");
    //bfs
    while(!q.empty()) {
        Job currentJob = q.front();
        q.pop();
        //Process current chunk
        if(visited[currentJob.pos.x & WORLDSIZE_MASK][currentJob.pos.y & WORLDSIZE_MASK][currentJob.pos.z & WORLDSIZE_MASK]) continue;
        visited[currentJob.pos.x & WORLDSIZE_MASK][currentJob.pos.y & WORLDSIZE_MASK][currentJob.pos.z & WORLDSIZE_MASK] = true;
        Chunk* currentChunk = getChunkCC(currentJob.pos); //used later inside neighbor loop
        if(currentChunk != nullptr) {
            Debugger::pushMark("Deferred rebuild", "Time spent rebuilding chunk geometry");
            currentChunk->rebuildMesh();
            Debugger::popMark();
            chunksToDraw.push_back(currentChunk);
        }
        int distance = currentJob.distance+1; //neighbor chunks's bfs distance
        //foreach face
        for(int i = 0; i < 6; ++i) {
            Job neighborJob = {currentJob.pos + offsets[i], distance};
            //out-of-bounds culling (null column, we do explore null chunks since they may be anywhere)
            if((neighborJob.pos.y >= (int)highestChunkY && faces[i] == Chunk::MAXY) || (getColumnCC(neighborJob.pos) == nullptr && renderer->getMode() == DeferredContainer::Deferred)) continue;
            //manhattan culling
            if(distance > Utils::manhattanDistance(initialChunkPos, neighborJob.pos)) continue;
            //visibility culling
            if(currentChunk != nullptr && !currentChunk->visibilityTest(faces[i])) continue;
            //fustrum culling
            colliderSphere.center = neighborJob.pos*CHUNKSIZE+colliderOffset;
            if(!Collision::intersects(cam->getFrustum(), colliderSphere)) continue;
            //push it
            Chunk* neighborChunk = getChunkCC(neighborJob.pos);
            if(neighborChunk != nullptr) neighborChunk->facesVisited.set(Chunk::getOppositeFace(faces[i]));
            q.push(neighborJob);
        }
    }
    Debugger::popMark(); //BFS time
    Debugger::pushMark("Deferred Batching", "Time spent actually sending chunk geometry to the GPU");
    //setup shaders for batching
    VBE_ASSERT_SIMPLE(Programs.exists("deferredChunk"));
    Programs.get("deferredChunk").uniform("V")->set(cam->getView());
    Programs.get("deferredChunk").uniform("VP")->set(cam->projection*cam->getView());
    Programs.get("deferredChunk").uniform("diffuseTex")->set(Textures2D.get("blocks"));
    sendDrawCommands(chunksToDraw, Programs.get("deferredChunk"));
    Debugger::popMark(); //batching
}

void World::sendDrawCommands(const std::list<Chunk*>& chunksToDraw, const ShaderProgram& program) const {
    int numDrawn = 0;
    Sun* sun = ((Sun*)getGame()->getObjectByName("sun"));
    static std::vector<vec3i> transforms(400);
    int batchCount = 0;
    //batched drawing
    MeshBatched::startBatch();
    for(Chunk* c : chunksToDraw) {
        if(c != nullptr && batchChunk(c, transforms, batchCount, sun)) {
            ++numDrawn;
            ++batchCount;
        }
        //submit batch
        if(batchCount == 400) {
            batchCount -= 400;
            program.uniform("transforms")->set(transforms);
            MeshBatched::endBatch();
            MeshBatched::startBatch();
        }
    }
    //clear up any pending chunks (last batch)
    if(batchCount > 0) {
        program.uniform("transforms")->set(transforms);
    }
    MeshBatched::endBatch();
    Debugger::numChunksDrawn = numDrawn;
}

bool World::batchChunk(Chunk* c, std::vector<vec3i>& transforms, const int batchCount, const Sun* sun) const {
    static Sphere colliderSphere(vec3f(0.0f), (CHUNKSIZE>>1)*1.74f);
    static vec3i colliderOffset = vec3i(CHUNKSIZE >> 1);
    //reset faces for next bfs
    c->facesVisited.reset();
    //even more culling for the shadowmap: outside of the player frustum and not between sun and frustum
    if(renderer->getMode() == DeferredContainer::ShadowMap) {
        const Frustum& f = ((Camera*)getGame()->getObjectByName("playerCam"))->getFrustum();
        colliderSphere.center = c->getAbsolutePos()+colliderOffset;
        bool pass = false;
        for(int i = 0; i < 6; ++i) {
            Plane p = f.getPlane((Frustum::PlaneID)i);
            if(!p.inside(colliderSphere) && glm::dot(p.n, sun->getDirection()) > 0)
                pass = true;
        }
        if(pass) return false;
    }
    //batch it
    if(c->hasMesh()) {
        c->draw();
        transforms[batchCount] = c->getAbsolutePos();
        return true;
    }
    return false;
}
