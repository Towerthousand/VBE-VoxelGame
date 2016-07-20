#include "World.hpp"
#include "Column.hpp"
#include "Chunk.hpp"
#include "../DeferredContainer.hpp"
#include "../Debugger.hpp"
#include "../Manager.hpp"
#include "Sun.hpp"
#include <cstring>
#include "DeferredCubeLight.hpp"

#define EPSILON 0.000001f

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

enum AngleOverlap {
    INVALID = -1,
    NONE = 0,
    PARTIAL,
    CONTAINS
};

struct Angle {
    vec3f dir;
    float halfAngle; //stored as tan(half angle)
    bool full;
};

vec3i offsets[6] = {
    {-1, 0, 0},
    { 1, 0, 0},
    { 0,-1, 0},
    { 0, 1, 0},
    { 0, 0,-1},
    { 0, 0, 1}
};

AngleOverlap overlapTest(const Angle& a, const Angle& b) {
    if(a.full) return CONTAINS;
    if(b.full) return NONE;
    vec3f dir1 = b.dir;
    vec3f dir2 = a.dir;
    // if dir1 == dir2 then just check the halfangles
    if(glm::epsilonEqual(dir1, dir2, EPSILON) == vec3b(true)) {
        if(a.halfAngle >= b.halfAngle)
            return CONTAINS;
        return NONE;
    }
    // side is the vector that orthogonally points away from dir1. With a
    // magnitude of halfangle tangent
    vec3f side = glm::cross(glm::normalize(glm::cross(dir1, dir2)), dir1)*b.halfAngle;
    // p1 and p2 are the "extremes" of b's angle
    vec3f p1 = dir1-side;
    vec3f p2 = dir1+side;
    // l1 and l2 are the length of the vector that results from
    // projecting p1 and p2 onto dir2
    float l1 = glm::dot(p1, dir2);
    float l2 = glm::dot(p2, dir2);
    // r1 and r2 are the rejection vectors of said projection
    float r1 = glm::length(p1-dir2*l1);
    float r2 = glm::length(p2-dir2*l2);
    // Return value explanation
    // NONE: this angle doesn't contain any half of the other,
    // so it can both mean it's fully inside it or that they don't
    // overlap at all
    // PARTIAL: this angle contains one of the ends of the other
    // or is right next to the other by a negligible distance
    // CONTAINS: this angle fully contains the other
    int result = 0;
    if(l1 > 0.0f && r1/l1 < a.halfAngle+EPSILON) result++;
    if(l2 > 0.0f && r2/l2 < a.halfAngle+EPSILON) result++;
    return static_cast<AngleOverlap>(result);
};

Angle angleUnion(const Angle& a, const Angle& b) {
    // if any of both are full, union will be full
    if(a.full || b.full)
        return {{0.0f, 0.0f, 0.0f}, 0.0f, true};
    // if one of them is null, return the other
    if(a.halfAngle == 0.0f)
        return b;
    if(b.halfAngle == 0.0f)
        return a;
    // if a contains b, result is a
    AngleOverlap acb = INVALID;
    if(a.halfAngle >= b.halfAngle) {
        acb = overlapTest(a, b);
        if(acb == CONTAINS)
            return a;
    }
    // and viceversa
    AngleOverlap bca = INVALID;
    if(b.halfAngle >= a.halfAngle) {
        bca = overlapTest(b, a);
        if(bca == CONTAINS)
            return b;
    }
    // General case. We compute the ends of the intersection
    // by rotating each cone direction away from the other direction
    // by their own half angle. This is done avoiding trigonometry,
    vec3f dir1 = a.dir;
    vec3f dir2 = b.dir;
    // if dir1 == dir2 then just check the halfangles
    if(glm::epsilonEqual(dir1, dir2, EPSILON) == vec3b(true)) {
        if(a.halfAngle > b.halfAngle)
            return a;
        return b;
    }
    // cross will point opposite directions depending on whether
    // dir1 is on the clockwise side of dir2 or the other way around,
    // in the context of the plane given by (0,0,0), dir1 and dir2,
    // where dir1 and dir2 are coplanar. cross will point orthogonally away
    // from that plane one side or the other
    vec3f cross = glm::normalize(glm::cross(dir1, dir2));
    // dir3 is obtained by rotating dir1 away from dir2 by a's halfangle.
    // the cross product of (cross, dir1) tells us which direction is
    // towards dir2.
    vec3f dir3 = glm::normalize(-glm::cross(cross, dir1)*a.halfAngle+dir1);
    // dir4 is obtained by rotating dir2 away from dir1 by b's halfangle.
    // the cross product of (-cross, dir2) tells us which direction is
    // towards dir1. We negate cross because -cross(dir1, dir2) is the same
    // as cross(dir2, dir1)
    vec3f dir4 = glm::normalize(-glm::cross(-cross, dir2)*b.halfAngle+dir2);
    // d is the direction of the union angle
    vec3f d = glm::normalize(dir3 + dir4);
    // if dot(dir1+dir2, d) < 0.0f, the union angle is > 180 deg
    if(glm::dot(dir1+dir2, d) <= 0.0f) return {{0.0f, 0.0f, 0.0f}, 0.0f, true};
    // we compute the tangent of the new halfangle by scaling one of the
    // outer vectors by the inverse of it's projection onto the new
    // angle's direction, and computing the length of the vector that
    // results from going from the central direction to this scaled outer direction
    return {d, glm::length(d-(dir3/glm::dot(dir3,d))), false};
}

Angle angleIntersection(const Angle& a, const Angle& b) {
    // if b is full, intersection will be a
    if(b.full)
        return {a.dir, a.halfAngle, a.full};
    // if a is full, intersection will be b
    if(a.full)
        return {b.dir, b.halfAngle, b.full};
    // if any of both are null, intersection will be empty
    if(a.halfAngle == 0.0f || b.halfAngle == 0.0f)
        return {{0.0f, 0.0f, 0.0f}, 0.0f, false};
    AngleOverlap acb = INVALID;
    // if a contains b, intersection will equal b
    if(a.halfAngle >= b.halfAngle) {
        acb = overlapTest(a, b);
        if(acb == CONTAINS)
            return {b.dir, b.halfAngle, b.full};
    }
    // if a is not bigger than b...
    if(acb == INVALID) {
        AngleOverlap bca = overlapTest(b, a);
        // b contains a, return a
        if(bca == CONTAINS)
            return {a.dir, a.halfAngle, a.full};
        // they don't overlap, return empty
        if(bca == NONE)
            return {{0.0f, 0.0f, 0.0f}, 0.0f, false};
    }
    else if(acb == NONE)
        // if a is bigger than B but does not contain it, return empty
        return {{0.0f, 0.0f, 0.0f}, 0.0f, false};
    // General case. We compute the ends of the intersection
    // by rotating each cone direction towards the other direction
    // by their own half angle. This is done avoiding trigonometry,
    // if a.dir == b.dir then just check the halfangles
    if(glm::epsilonEqual(a.dir, b.dir, EPSILON) == vec3b(true)) {
        if(a.halfAngle > b.halfAngle)
            return b;
        return a;
    }
    // cross will point opposite directions depending on whether
    // a.dir is on the clockwise side of b.dir or the other way around,
    // in the context of the plane given by (0,0,0), a.dir and b.dir,
    // where a.dir and b.dir are coplanar. cross will point orthogonally away
    // from that plane one side or the other
    vec3f cross = glm::normalize(glm::cross(a.dir, b.dir));
    // dir3 is obtained by rotating a.dir towards b.dir by a's halfangle.
    // the cross product of (cross, a.dir) tells us which direction is
    // towards b.dir.
    vec3f dir3 = glm::normalize(glm::cross(cross, a.dir)*a.halfAngle+a.dir);
    // dir4 is obtained by rotating b.dir towards a.dir by b's halfangle.
    // the cross product of (-cross, b.dir) tells us which direction is
    // towards a.dir. We negate cross because -cross(a.dir, b.dir) is the same
    // as cross(b.dir, a.dir)
    vec3f dir4 = glm::normalize(glm::cross(-cross, b.dir)*b.halfAngle+b.dir);
    // d is the direction of the intersection angle
    vec3f d = glm::normalize(dir3 + dir4);
    // we compute the tangent of the new halfangle by scaling one of the
    // outer vectors by the inverse of it's projection onto the new
    // angle's direction, and computing the length of the vector that
    // results from going from the central direction to this scaled outer direction
    float tangent = glm::length(d-(dir3/glm::dot(dir3,d)));
    // this handles the imprecision-caused edge case where an angle is created with
    // a very small tangent
    if(glm::epsilonEqual(tangent, 0.0f, EPSILON))
        return {{0.0f, 0.0f, 0.0f}, 0.0f, false};
    return {d, tangent, false};
}

// This is a standard Fisher-Yates random in-place shuffle
template<typename T>
void fy_shuffle(T* v, int count) {
    for(int i = count-1; i > 0; --i) {
        int j = rand()%i;
        std::swap(v[i], v[j]);
    }
}

bool equals(const vec3f& a, const vec3f& b) {
    return glm::epsilonEqual(a, b, EPSILON) == vec3b(true);
}

Angle getCone(const vec3f& p1, const vec3f& p2) {
    // p1 and p2 are assumed to be unit vectors
    vec3f dir = glm::normalize(p1+p2);
    float dist = glm::dot(p1, dir);
    float tan = glm::distance(p1, dir*dist);
    return {dir, tan/dist, false};
}

Angle getCone(const vec3f& p1, const vec3f& p2, const vec3f& p3) {
    // The idea is to compute the two planes that run in between p1,p2 and p2,p3.
    // The direction of the cone will be the intersection of those planes (which
    // happens to be a line) and the radius can be then computed using
    // any of the three original vectors.
    vec3f pn1 =
            glm::cross(
                    p1+p2,
                    glm::cross(p1, p2)
                );
    vec3f pn2 =
            glm::cross(
                    p2+p3,
                    glm::cross(p2, p3)
                );
    // Cross product of the two plane's normals will give us the new direction
    vec3f dir = glm::normalize(glm::cross(pn1, pn2));
    // Flip the direction in case we got it the wrong way.
    if(glm::dot(dir, p1) <= 0.0f)
        dir = -dir;
    // Compute the new cone angle
    float dist = glm::dot(p1, dir);
    float tan = glm::distance(p1, dir*dist);
    return {dir, tan/dist, false};
}

bool insideCone(const Angle& c, const vec3f& v) {
    float dist = glm::dot(v, c.dir);
    float tan = glm::distance(v, c.dir*dist);
    return (dist > 0.0f && tan/dist <= (c.halfAngle+EPSILON));
}

// This is the unroll implementation for the algorithm found at
// http://www.cs.technion.ac.il/~cggc/files/gallery-pdfs/Barequet-1.pdf
// which is an application of the minimum enclosing circle problem to
// the bounding cone problem.  All vectors in "points" are assumed to
// be unit vectors
//
// I left the recursive calls commented wherever they would
// be called for the sake of clarity/readability.
// This only works with four points, not for the generic case.
Angle minConeUnroll(const vec3f& v0, const vec3f& v1, const vec3f& v2, const vec3f& v3) {
    // c = minCone(p);
    Angle c = getCone(v0, v1);
    if(!insideCone(c, v2)) {
        //c = minConeOnePointUnroll(p, 2, v2);
        c = getCone(v2, v0);
        if(!insideCone(c, v1)) {
            //c = minConeTwoPoint(p, 1, v2, v1);
            c = getCone(v2, v1);
            if(!insideCone(c, v0))
                c = getCone(v2, v1, v0);
        }
    }
    if(!insideCone(c, v3)) {
        //c = minConeOnePointUnroll(p, 3, v3);
        c = getCone(v3, v0);
        if(!insideCone(c, v1)) {
            //c = minConeTwoPoint(p, 1, v3, v1);
            c = getCone(v3, v1);
            if(!insideCone(c, v0))
                c = getCone(v3, v1, v0);
        }
        if(!insideCone(c, v2)) {
            //c = minConeTwoPoint(p, 2, v3, v2);
            c = getCone(v3, v2);
            if(!insideCone(c, v0))
                c = getCone(v3, v2, v0);
            if(!insideCone(c, v1))
                c = getCone(v3, v2, v1);
        }
    }
    return c;
}

// If genMode2D is true, this will calculate the new cones using only
// two points per face instead of 4, hence simulating a 2D grid case
Angle getAngle(const vec3i& pos, Chunk::Face f,const vec3i& origin) {
    if(pos == origin)
        return {{0.0f, 0.0f, 0.0f}, 0.0f, true};
    vec3f center = vec3f(pos)+0.5f+vec3f(offsets[f])*0.5f;
    vec3f orig = vec3f(origin)+0.5f;
    static vec3f p[4];
    vec3f rel = center-orig;
    switch(f) {
        case Chunk::MINX:
        case Chunk::MAXX:
            p[0] = rel+vec3f( 0.0f, 0.5f, 0.5f);
            p[1] = rel+vec3f( 0.0f,-0.5f, 0.5f);
            p[2] = rel+vec3f( 0.0f, 0.5f,-0.5f);
            p[3] = rel+vec3f( 0.0f,-0.5f,-0.5f);
            break;
        case Chunk::MINY:
        case Chunk::MAXY:
            p[0] = rel+vec3f( 0.5f, 0.0f, 0.5f);
            p[1] = rel+vec3f(-0.5f, 0.0f, 0.5f);
            p[2] = rel+vec3f( 0.5f, 0.0f,-0.5f);
            p[3] = rel+vec3f(-0.5f, 0.0f,-0.5f);
            break;
        case Chunk::MINZ:
        case Chunk::MAXZ:
            p[0] = rel+vec3f( 0.5f, 0.5f, 0.0f);
            p[1] = rel+vec3f(-0.5f, 0.5f, 0.0f);
            p[2] = rel+vec3f( 0.5f,-0.5f, 0.0f);
            p[3] = rel+vec3f(-0.5f,-0.5f, 0.0f);
            break;
        default:
            VBE_ASSERT_SIMPLE(f != Chunk::ALL_FACES);
    }
    for(vec3f& v : p) v = glm::normalize(v);
    fy_shuffle(p, 4);
    return minConeUnroll(p[0], p[1], p[2], p[3]);
}

Angle angles[WORLDSIZE][WORLDSIZE][WORLDSIZE];
bool visited[WORLDSIZE][WORLDSIZE][WORLDSIZE];

void World::drawPlayerCam(const Camera* cam) const{
    int skippedChunks = 0;
    VBE_ASSERT_SIMPLE(renderer->getMode() == DeferredContainer::Deferred);
    std::list<Chunk*> chunksToDraw; //push all the chunks that must be drawn here
    vec3i initialChunkPos(vec3i(glm::floor(cam->getWorldPos())) >> CHUNKSIZE_POW2);
    std::queue<vec3i> q; //bfs queue, each node is (entry face, chunkPos, distance to source), in chunk coords
    //mark all faces of init node as visible
    Chunk* initialChunk = getChunkCC(initialChunkPos); //used later inside neighbor loop
    if(initialChunk != nullptr) for(int i = 0; i < 6; ++i) initialChunk->facesVisited.set(i);
    //collider
    Sphere colliderSphere(vec3f(0.0f), (CHUNKSIZE >> 1)*1.74f);
    vec3i colliderOffset = vec3i(CHUNKSIZE >> 1);
    //memset stuff to 0
    memset(visited, 0, sizeof(visited));
    memset(angles, 0, sizeof(angles));
    //push chunk
    q.push(initialChunkPos);
    angles[initialChunkPos.x & WORLDSIZE_MASK][initialChunkPos.y & WORLDSIZE_MASK][initialChunkPos.z & WORLDSIZE_MASK].full = true;
    //bfs
    Debugger::pushMark("Deferred BFS", "CPU BFS for the deferred pass");
    while(!q.empty()) {
        vec3i current = q.front();
        q.pop();
        //Process current chunk
        if(visited[current.x & WORLDSIZE_MASK][current.y & WORLDSIZE_MASK][current.z & WORLDSIZE_MASK]) continue;
        visited[current.x & WORLDSIZE_MASK][current.y & WORLDSIZE_MASK][current.z & WORLDSIZE_MASK] = true;
        const Angle& currentAngle = angles[current.x & WORLDSIZE_MASK][current.y & WORLDSIZE_MASK][current.z & WORLDSIZE_MASK];
        Chunk* currentChunk = getChunkCC(current); //used later inside neighbor loop
        if(!currentAngle.full && currentAngle.halfAngle == 0.0f) {
            if(currentChunk != nullptr) skippedChunks++;
            continue;
        }
        if(currentChunk != nullptr) {
            Debugger::pushMark("Geometry rebuild", "Time spent rebuilding chunk geometry");
            currentChunk->rebuildMesh();
            Debugger::popMark();
            chunksToDraw.push_back(currentChunk);
        }
        //foreach face
        for(int i = 0; i < 6; ++i) {
            vec3i neighbor = current + offsets[i];
            //out-of-bounds culling (null column, we do explore null chunks since they may be anywhere)
            if((neighbor.y >= (int)highestChunkY && (Chunk::Face)i == Chunk::MAXY) || getColumnCC(neighbor) == nullptr) continue;
            //manhattan culling
            if(Utils::manhattanDistance(initialChunkPos, neighbor) > Utils::manhattanDistance(initialChunkPos, neighbor)) continue;
            //visibility culling
            if(currentChunk != nullptr && !currentChunk->visibilityTest((Chunk::Face)i)) continue;
            //fustrum culling
            colliderSphere.center = neighbor*CHUNKSIZE+colliderOffset;
            if(!Collision::intersects(cam->getFrustum(), colliderSphere)) continue;
            //mark face as visited
            Chunk* neighborChunk = getChunkCC(neighbor);
            if(neighborChunk != nullptr) neighborChunk->facesVisited.set(Chunk::getOppositeFace((Chunk::Face)i));
            //update neighbor angle
            Debugger::pushMark("Angle Calculation", "Time spent recalculating visibility angles");
            Angle& na = angles[neighbor.x & WORLDSIZE_MASK][neighbor.y & WORLDSIZE_MASK][neighbor.z & WORLDSIZE_MASK];
            na = angleUnion(na, angleIntersection(getAngle(current, (Chunk::Face)i, initialChunkPos), currentAngle));
            Debugger::popMark();
            //push it
            q.push(neighbor);
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
    Debugger::numChunksSkipped = skippedChunks;
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
