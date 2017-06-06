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
    generator.lock();
    generator.discardGenerateTasks();
    Column* newCol = nullptr;
    while((newCol = generator.pullDone()) != nullptr) {
        if(getColumnCC(newCol->getX(), 0, newCol->getZ()) != nullptr)
            generator.unloadColumn(newCol);
        else
            columns[newCol->getX()&WORLDSIZE_MASK][newCol->getZ()&WORLDSIZE_MASK] = newCol;
    }
    Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");
    vec2i playerChunkPos = vec2i(cam->getWorldPos().x,cam->getWorldPos().z) >> CHUNKSIZE_POW2;
    std::vector<std::pair<float,std::pair<int,int> > > tasks;
    AABB bounds = AABB();
    chunksExist = false;
    for(int x = -WORLDSIZE/2; x < WORLDSIZE/2; ++x)
        for(int z = -WORLDSIZE/2; z < WORLDSIZE/2; ++z) {
            vec2i colPos = vec2i(playerChunkPos) + vec2i(x,z);
            Column* actual = getColumnCC(colPos.x,0,colPos.y);
            if(actual == nullptr) {
                Column*& realpos = columns[colPos.x&WORLDSIZE_MASK][colPos.y&WORLDSIZE_MASK];
                if(realpos != nullptr) generator.unloadColumn(realpos);
                tasks.push_back(std::pair<float,std::pair<int,int> >(glm::length(vec2f(x, z)),std::pair<int,int>(colPos.x,colPos.y)));
                realpos = nullptr;
                continue;
            }
            for(unsigned int y = 0; y < actual->getChunkCount(); ++y){
                Chunk* c = actual->getChunkCC(y);
                if(c == nullptr) continue;
                c->update(deltaTime);
                bounds.extend(vec3f(c->getAbsolutePos()));
                chunksExist = true;
            }
        }
    generator.setRelevantArea(playerChunkPos-WORLDSIZE/2, playerChunkPos+WORLDSIZE/2-1);
    std::sort(tasks.begin(),tasks.end());
    for(unsigned int i = 0; i < tasks.size(); ++i)
        generator.queueLoad(vec2i(tasks[i].second.first,tasks[i].second.second));
    generator.update();
    generator.unlock();
    minLoadedCoords = vec3i(bounds.getMin()) / CHUNKSIZE;
    maxLoadedCoords = vec3i(bounds.getMax()) / CHUNKSIZE;
    Debugger::popMark();
}

void World::draw() const {
    if(!chunksExist) return;
    switch(renderer->getMode()) {
        case DeferredContainer::Deferred: {
            Debugger::pushMark("Player Cam World Draw", "Time spent drawing chunks from the player camera");

            // get the player cam
            Camera* pCam = (Camera*)getGame()->getObjectByName("playerCam");

            // get the list of chunks to draw
            std::list<Chunk*> chunksToDraw = getPerspectiveCamChunks(pCam);

            // setup shaders for batching
            VBE_ASSERT_SIMPLE(Programs.exists("deferredChunk"));
            Programs.get("deferredChunk").uniform("V")->set(pCam->getView());
            Programs.get("deferredChunk").uniform("VP")->set(pCam->projection*pCam->getView());
            Programs.get("deferredChunk").uniform("diffuseTex")->set(Textures2D.get("blocks"));

            // send commands
            unsigned int numDrawn = sendDrawCommands(chunksToDraw, Programs.get("deferredChunk"));
            Debugger::numChunksDrawn = numDrawn;
            Debugger::popMark(); //world draw
            break;
        }
        case DeferredContainer::TransShadowMap:
        case DeferredContainer::ShadowMap: {
            Debugger::pushMark("Sun Cam World Draw", "Time spent drawing chunks from the sun camera");

            // update sun cameras
            Sun* sun = (Sun*)getGame()->getObjectByName("sun");
            sun->updateCameras();

            // get the list of chunks to draw
            std::list<Chunk*> chunksToDraw = getSunCamChunks(sun->getGlobalCam());

            //setup shaders for batching
            VBE_ASSERT_SIMPLE(Programs.exists("depthShader"));
            Programs.get("depthShader").uniform("VP")->set(sun->getVPMatrices());

            // send commands
            unsigned int numDrawn = sendDrawCommands(chunksToDraw, Programs.get("depthShader"));
            Debugger::numChunksDrawnShadow = numDrawn;
            Debugger::popMark(); //world draw
            break;
        }
        case DeferredContainer::Forward: {
            Debugger::pushMark("Forward World Draw", "Time spent drawing transparent geometry");

            // get the player cam
            Camera* pCam = (Camera*)getGame()->getObjectByName("playerCam");
            Sun* sun = (Sun*)getGame()->getObjectByName("sun");

            // get the list of chunks to draw (reversed for correct alpha blending)
            std::list<Chunk*> chunksToDraw = getPerspectiveCamChunks(pCam);
            chunksToDraw.reverse();

            // setup shaders for batching
            VBE_ASSERT_SIMPLE(Programs.exists("forwardChunk"));
            //compute each of the cascaded cameras's matrices
            glm::mat4 biasMatrix( //gets coords from [-1..1] to [0..1]
                                  0.5, 0.0, 0.0, 0.0,
                                  0.0, 0.5, 0.0, 0.0,
                                  0.0, 0.0, 0.5, 0.0,
                                  0.5, 0.5, 0.5, 1.0
                                  );
            std::vector<mat4f> depthMVP(NUM_SUN_CASCADES);
            for(int i = 0; i < NUM_SUN_CASCADES; ++i)
                depthMVP[i] = biasMatrix*(sun->getVPMatrices()[i]*fullTransform);
            const RenderTargetBase* screen = RenderTargetBase::getCurrent();
            Programs.get("forwardChunk").uniform("V")->set(pCam->getView());
            Programs.get("forwardChunk").uniform("VP")->set(pCam->projection*pCam->getView());
            Programs.get("forwardChunk").uniform("diffuseTex")->set(Textures2D.get("blocks"));
            Programs.get("forwardChunk").uniform("camMV")->set(pCam->getView()*fullTransform);
            Programs.get("forwardChunk").uniform("invResolution")->set(vec2f(1.0f/screen->getSize().x, 1.0f/screen->getSize().y));
            Programs.get("forwardChunk").uniform("invCamProj")->set(glm::inverse(pCam->projection));
            Programs.get("forwardChunk").uniform("invCamView")->set(glm::inverse(pCam->getView()));
            Programs.get("forwardChunk").uniform("lightDir")->set(sun->getCam(0)->getForward());
            Programs.get("forwardChunk").uniform("worldsize")->set(WORLDSIZE);
            Programs.get("forwardChunk").uniform("depthMVP")->set(depthMVP);
            Programs.get("forwardChunk").uniform("depthPlanes")->set(sun->getDepthPlanes());
            Programs.get("forwardChunk").uniform("sunDepthTrans")->set(renderer->getTransSunDepth());
            Programs.get("forwardChunk").uniform("sunDepth")->set(renderer->getSunDepth());

            // send commands
            unsigned int numDrawn = sendDrawCommands(chunksToDraw, Programs.get("forwardChunk"));
            Debugger::numChunksDrawn = numDrawn;
            Debugger::popMark(); //world draw
            break;
        }
        default: break;
    }
}

namespace {
    enum ConeOverlap {
        INVALID = -1,
        NONE = 0,
        PARTIAL,
        CONTAINS
    };

    struct Cone {
        vec3f dir;
        float halfCone; //stored as tan(half angle)
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

    ConeOverlap overlapTest(const Cone& a, const Cone& b) {
        if(a.full) return CONTAINS;
        if(b.full) return NONE;
        vec3f dir1 = b.dir;
        vec3f dir2 = a.dir;
        // if dir1 == dir2 then just check the halfangles
        if(glm::epsilonEqual(dir1, dir2, EPSILON) == vec3b(true)) {
            if(a.halfCone >= b.halfCone)
                return CONTAINS;
            return NONE;
        }
        // side is the vector that orthogonally points away from dir1. With a
        // magnitude of halfangle tangent
        vec3f side = glm::cross(glm::normalize(glm::cross(dir1, dir2)), dir1)*b.halfCone;
        // p1 and p2 are the "extremes" of b's cone
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
        // NONE: this cone doesn't contain any half of the other,
        // so it can both mean it's fully inside it or that they don't
        // overlap at all
        // PARTIAL: this cone contains one of the ends of the other
        // or is right next to the other by a negligible distance
        // CONTAINS: this cone fully contains the other
        int result = 0;
        if(l1 > 0.0f && r1/l1 < a.halfCone+EPSILON) result++;
        if(l2 > 0.0f && r2/l2 < a.halfCone+EPSILON) result++;
        return static_cast<ConeOverlap>(result);
    };

    Cone coneUnion(const Cone& a, const Cone& b) {
        // if any of both are full, union will be full
        if(a.full || b.full)
            return {{0.0f, 0.0f, 0.0f}, 0.0f, true};
        // if one of them is null, return the other
        if(a.halfCone == 0.0f)
            return b;
        if(b.halfCone == 0.0f)
            return a;
        // if a contains b, result is a
        ConeOverlap acb = INVALID;
        if(a.halfCone >= b.halfCone) {
            acb = overlapTest(a, b);
            if(acb == CONTAINS)
                return a;
        }
        // and viceversa
        ConeOverlap bca = INVALID;
        if(b.halfCone >= a.halfCone) {
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
            if(a.halfCone > b.halfCone)
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
        vec3f dir3 = glm::normalize(-glm::cross(cross, dir1)*a.halfCone+dir1);
        // dir4 is obtained by rotating dir2 away from dir1 by b's halfangle.
        // the cross product of (-cross, dir2) tells us which direction is
        // towards dir1. We negate cross because -cross(dir1, dir2) is the same
        // as cross(dir2, dir1)
        vec3f dir4 = glm::normalize(-glm::cross(-cross, dir2)*b.halfCone+dir2);
        // d is the direction of the union cone
        vec3f d = glm::normalize(dir3 + dir4);
        // if dot(dir1+dir2, d) < 0.0f, the union cone is > 180 deg
        if(glm::dot(dir1+dir2, d) <= 0.0f) return {{0.0f, 0.0f, 0.0f}, 0.0f, true};
        // we compute the tangent of the new halfangle by scaling one of the
        // outer vectors by the inverse of it's projection onto the new
        // cone's direction, and computing the length of the vector that
        // results from going from the central direction to this scaled outer direction
        return {d, glm::length(d-(dir3/glm::dot(dir3,d))), false};
    }

    Cone coneIntersection(const Cone& a, const Cone& b) {
        // if b is full, intersection will be a
        if(b.full)
            return {a.dir, a.halfCone, a.full};
        // if a is full, intersection will be b
        if(a.full)
            return {b.dir, b.halfCone, b.full};
        // if any of both are null, intersection will be empty
        if(a.halfCone == 0.0f || b.halfCone == 0.0f)
            return {{0.0f, 0.0f, 0.0f}, 0.0f, false};
        ConeOverlap acb = INVALID;
        // if a contains b, intersection will equal b
        if(a.halfCone >= b.halfCone) {
            acb = overlapTest(a, b);
            if(acb == CONTAINS)
                return {b.dir, b.halfCone, b.full};
        }
        // if a is not bigger than b...
        if(acb == INVALID) {
            ConeOverlap bca = overlapTest(b, a);
            // b contains a, return a
            if(bca == CONTAINS)
                return {a.dir, a.halfCone, a.full};
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
            if(a.halfCone > b.halfCone)
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
        vec3f dir3 = glm::normalize(glm::cross(cross, a.dir)*a.halfCone+a.dir);
        // dir4 is obtained by rotating b.dir towards a.dir by b's halfangle.
        // the cross product of (-cross, b.dir) tells us which direction is
        // towards a.dir. We negate cross because -cross(a.dir, b.dir) is the same
        // as cross(b.dir, a.dir)
        vec3f dir4 = glm::normalize(glm::cross(-cross, b.dir)*b.halfCone+b.dir);
        // d is the direction of the intersection cone
        vec3f d = glm::normalize(dir3 + dir4);
        // we compute the tangent of the new halfangle by scaling one of the
        // outer vectors by the inverse of it's projection onto the new
        // cone's direction, and computing the length of the vector that
        // results from going from the central direction to this scaled outer direction
        float tangent = glm::length(d-(dir3/glm::dot(dir3,d)));
        // this handles the imprecision-caused edge case where a cone is created with
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

    Cone getCone(const vec3f& p1, const vec3f& p2) {
        // p1 and p2 are assumed to be unit vectors
        vec3f dir = glm::normalize(p1+p2);
        float dist = glm::dot(p1, dir);
        float tan = glm::distance(p1, dir*dist);
        return {dir, tan/dist, false};
    }

    Cone getCone(const vec3f& p1, const vec3f& p2, const vec3f& p3) {
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

    bool insideCone(const Cone& c, const vec3f& v) {
        float dist = glm::dot(v, c.dir);
        float tan = glm::distance(v, c.dir*dist);
        return (dist > 0.0f && tan/dist <= (c.halfCone+EPSILON));
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
    Cone minConeUnroll(const vec3f& v0, const vec3f& v1, const vec3f& v2, const vec3f& v3) {
        // c = minCone(p);
        Cone c = getCone(v0, v1);
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

    Cone getCone(const vec3i& pos, Chunk::Face f,const vec3i& origin) {
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

    struct Square {
        vec2f p;
        vec2f d;
    };

    //pair(float, vector) comp for pqueue
    struct fvpaircomp {
        bool operator() (const std::pair<float, vec3i>& lhs, const std::pair<float, vec3i>& rhs) const {
            return lhs.first>rhs.first;
        };
    };

    Square squareUnion(const Square& s1, const Square& s2) {
        if(s1.p == vec2f(0.0f) && s1.d == vec2f(0.0f)) return s2;
        else if(s2.p == vec2f(0.0f) && s2.d == vec2f(0.0f)) return s1;
        Square s = {
            {
                glm::min(s1.p.x, s2.p.x),
                glm::min(s1.p.y, s2.p.y),
            },
            {
                glm::max(s1.p.x + s1.d.x, s2.p.x + s2.d.x),
                glm::max(s1.p.y + s1.d.y, s2.p.y + s2.d.y),
            }
        };
        s.d -= s.p;
        if(glm::epsilonEqual(s.d.x*s.d.y, 0.0f, EPSILON)) return {{0.0f, 0.0f}, {0.0f, 0.0f}};
        return s;
    };

    Square squareIntersection(const Square& s1, const Square& s2) {
        if(s1.d == vec2f(0.0f) ||
           s2.d == vec2f(0.0f) ||
           s1.p.x+s1.d.x <= s2.p.x ||
           s1.p.x >= s2.p.x+s2.d.x ||
           s1.p.y+s1.d.y <= s2.p.y ||
           s1.p.y >= s2.p.y+s2.d.y)
            return {{0.0f, 0.0f}, {0.0f, 0.0f}};
        Square s = {
            {
                glm::max(s1.p.x, s2.p.x),
                glm::max(s1.p.y, s2.p.y),
            },
            {
                glm::min(s1.p.x + s1.d.x, s2.p.x + s2.d.x),
                glm::min(s1.p.y + s1.d.y, s2.p.y + s2.d.y),
            }
        };
        s.d -= s.p;
        if(glm::epsilonEqual(s.d.x*s.d.y, 0.0f, EPSILON)) return {{0.0f, 0.0f}, {0.0f, 0.0f}};
        VBE_ASSERT(s.d.x >= 0.0f && s.d.y >= 0.0f, "Sanity check for squareIntersection");
        return s;
    };

    mat3f getViewMatrixForDirection(const vec3f& direction) {
        vec3f dummyUp = (glm::abs(glm::normalize(direction)) == vec3f(0, 1, 0))? vec3f(0,0,1) : vec3f(0, 1, 0);
        vec3f front = glm::normalize(-direction);
        vec3f right = glm::normalize(glm::cross(dummyUp, front));
        vec3f up = glm::normalize(glm::cross(front, right));
        return glm::transpose(
            mat3f(
                right.x, right.y, right.z,
                up.x   , up.y   , up.z   ,
                front.x, front.y, front.z
            )
        );
    };

    Square getSquare(vec3i pos, Chunk::Face f, const mat3f& viewMatrix) {
        vec3f center = vec3f(pos)+0.5f+vec3f(offsets[f])*0.5f;
        std::vector<vec3f> p(4);
        switch(f) {
            case Chunk::MINX:
            case Chunk::MAXX:
                p[0] = center+vec3f( 0.0f, 0.5f, 0.5f);
                p[1] = center+vec3f( 0.0f,-0.5f, 0.5f);
                p[2] = center+vec3f( 0.0f, 0.5f,-0.5f);
                p[3] = center+vec3f( 0.0f,-0.5f,-0.5f);
                break;
            case Chunk::MINY:
            case Chunk::MAXY:
                p[0] = center+vec3f( 0.5f, 0.0f, 0.5f);
                p[1] = center+vec3f(-0.5f, 0.0f, 0.5f);
                p[2] = center+vec3f( 0.5f, 0.0f,-0.5f);
                p[3] = center+vec3f(-0.5f, 0.0f,-0.5f);
                break;
            case Chunk::MINZ:
            case Chunk::MAXZ:
                p[0] = center+vec3f( 0.5f, 0.5f, 0.0f);
                p[1] = center+vec3f(-0.5f, 0.5f, 0.0f);
                p[2] = center+vec3f( 0.5f,-0.5f, 0.0f);
                p[3] = center+vec3f(-0.5f,-0.5f, 0.0f);
                break;
            case Chunk::ALL_FACES:
                VBE_ASSERT_SIMPLE(f != Chunk::ALL_FACES);
        }
        vec2f min = vec2f(std::numeric_limits<float>::max());
        vec2f max = vec2f(std::numeric_limits<float>::lowest());
        for(vec3f& v : p) {
            v = viewMatrix*v;
            min.x = glm::min(min.x, v.x);
            min.y = glm::min(min.y, v.y);
            max.x = glm::max(max.x, v.x);
            max.y = glm::max(max.y, v.y);
        }
        return {min, max-min};
    }

    bool insideCoords(const vec3i &min, const vec3i&max, const vec3i& coords) {
        return (coords.x >= min.x &&
                coords.y >= min.y &&
                coords.z >= min.z &&
                coords.x <= max.x &&
                coords.y <= max.y &&
                coords.z <= max.z);
    };

    Cone cones[WORLDSIZE][WORLDSIZE][WORLDSIZE];
    bool visited[WORLDSIZE][WORLDSIZE][WORLDSIZE];
    Square squares[WORLDSIZE][WORLDSIZE][WORLDSIZE];
}

std::list<Chunk*> World::getPerspectiveCamChunks(const Camera* cam) const {
    int skippedChunks = 0;
    std::list<Chunk*> chunksToDraw; //push all the chunks that must be drawn here
    vec3i initialChunkPos(vec3i(glm::floor(cam->getWorldPos())) >> CHUNKSIZE_POW2);
    //bfs queue, each node is chunkPos, in chunk coord space
    std::queue<vec3i> q;
    //bounds for out-of bound-checking
    AABB bounds = AABB(minLoadedCoords, maxLoadedCoords);
    bounds.extend(initialChunkPos);
    vec3i minBounds = bounds.getMin();
    vec3i maxBounds = bounds.getMax();
    //memset stuff to 0
    memset(visited, 0, sizeof(visited));
    if(USE_CPU_VISIBILITY) memset(cones, 0, sizeof(cones));
    //collider
    Sphere colliderSphere(vec3f(0.0f), (CHUNKSIZE >> 1)*1.74f);
    vec3i colliderOffset = vec3i(CHUNKSIZE >> 1);
    //mark all faces of init node as visible
    Chunk* initialChunk = getChunkCC(initialChunkPos); //used later inside neighbor loop
    if(initialChunk != nullptr) for(int i = 0; i < 6; ++i) initialChunk->facesVisited.set(i);
    //push initial node
    q.push(initialChunkPos);
    if(USE_CPU_VISIBILITY)
        cones[initialChunkPos.x & WORLDSIZE_MASK][initialChunkPos.y & WORLDSIZE_MASK][initialChunkPos.z & WORLDSIZE_MASK].full = true;
    //bfs
    Debugger::pushMark("BFS", "CPU BFS to determine which chunks to draw");
    while(!q.empty()) {
        vec3i current = q.front();
        q.pop();
        //Process current chunk
        if(visited[current.x & WORLDSIZE_MASK][current.y & WORLDSIZE_MASK][current.z & WORLDSIZE_MASK]) continue;
        visited[current.x & WORLDSIZE_MASK][current.y & WORLDSIZE_MASK][current.z & WORLDSIZE_MASK] = true;
        const Cone& currentCone = cones[current.x & WORLDSIZE_MASK][current.y & WORLDSIZE_MASK][current.z & WORLDSIZE_MASK];
        Chunk* currentChunk = getChunkCC(current); //used later inside neighbor loop
        if(USE_CPU_VISIBILITY && !currentCone.full && currentCone.halfCone == 0.0f) {
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
            if(!insideCoords(minBounds, maxBounds, neighbor)) continue;
            //manhattan culling
            if(Utils::manhattanDistance(initialChunkPos, neighbor) > Utils::manhattanDistance(initialChunkPos, neighbor)) continue;
            //visibility culling
            if(currentChunk != nullptr && !currentChunk->visibilityTest((Chunk::Face)i) && currentChunk != initialChunk) continue;
            //fustrum culling
            colliderSphere.center = neighbor*CHUNKSIZE+colliderOffset;
            if(!Collision::intersects(cam->getFrustum(), colliderSphere)) continue;
            //mark face as visited
            Chunk* neighborChunk = getChunkCC(neighbor);
            if(neighborChunk != nullptr) neighborChunk->facesVisited.set(Chunk::getOppositeFace((Chunk::Face)i));
            //update neighbor cone
            if(USE_CPU_VISIBILITY) {
                Debugger::pushMark("Cone Calculation", "Time spent recalculating visibility cones");
                Cone& na = cones[neighbor.x & WORLDSIZE_MASK][neighbor.y & WORLDSIZE_MASK][neighbor.z & WORLDSIZE_MASK];
                na = coneUnion(na, coneIntersection(getCone(current, (Chunk::Face)i, initialChunkPos), currentCone));
                Debugger::popMark();
            }
            //push it
            q.push(neighbor);
        }
    }
    if(renderer->getMode() == DeferredContainer::Deferred) Debugger::numChunksSkipped = skippedChunks;
    Debugger::popMark(); //BFS time
    return chunksToDraw;
}

std::list<Chunk*> World::getSunCamChunks(const Camera* cam) const {
    int skippedChunks = 0;
    std::list<Chunk*> chunksToDraw; //push all the chunks that must be drawn here
    //memset stuff to 0
    memset(visited, 0, sizeof(visited));
    if(USE_CPU_VISIBILITY) memset(squares, 0, sizeof(squares));
    //the queue
    std::priority_queue<std::pair<float, vec3i>, std::vector<std::pair<float, vec3i>>, fvpaircomp> q;
    //collider
    Sphere colliderSphere(vec3f(0.0f), (CHUNKSIZE >> 1)*1.74f);
    vec3i colliderOffset = vec3i(CHUNKSIZE >> 1);
    //since the shadow frustum is hacked to only be as deep as the view, to keep accurracy high,
    //chunks that are behind the near plane must be considered.
    std::bitset<6> ommitedPlanes;
    ommitedPlanes.set(Frustum::NEAR_PLANE);
    //sun stuff
    vec3f sunDir = cam->getForward();
    mat3f viewMatrix = getViewMatrixForDirection(sunDir);
    //push initial nodes
    Debugger::pushMark("BFS", "CPU BFS to determine which chunks to draw");
    Debugger::pushMark("Initial BFS Nodes", "Time spent finding and pushing BFS entry nodes");
    std::bitset<6> faces = 0;
    if(sunDir.x != 0.0f) faces.set(sunDir.x < 0.0f? Chunk::MAXX : Chunk::MINX);
    if(sunDir.y != 0.0f) faces.set(sunDir.y < 0.0f? Chunk::MAXY : Chunk::MINY);
    if(sunDir.z != 0.0f) faces.set(sunDir.z < 0.0f? Chunk::MAXZ : Chunk::MINZ);
    for(int i = 0; i < 6; ++i) {
        if(!faces.test(i)) continue;
        vec3i min = {
            (i == Chunk::MAXX) ? maxLoadedCoords.x : minLoadedCoords.x,
            (i == Chunk::MAXY) ? maxLoadedCoords.y : minLoadedCoords.y,
            (i == Chunk::MAXZ) ? maxLoadedCoords.z : minLoadedCoords.z
        };
        vec3i max = {
            (i == Chunk::MINX) ? minLoadedCoords.x : maxLoadedCoords.x,
            (i == Chunk::MINY) ? minLoadedCoords.y : maxLoadedCoords.y,
            (i == Chunk::MINZ) ? minLoadedCoords.z : maxLoadedCoords.z
        };
        for(int x = min.x; x <= max.x; ++x)
            for(int y = min.y; y <= max.y; ++y)
                for(int z = min.z; z <= max.z; ++z) {
                    vec3i coords = vec3i(x, y, z);
                    float len = glm::dot(vec3f(coords), sunDir);
                    //fustrum culling
                    colliderSphere.center = coords*CHUNKSIZE+colliderOffset;
                    if(!Collision::intersects(cam->getFrustum(), colliderSphere, ommitedPlanes)) continue;
                    //mark face as visited
                    Chunk* c = getChunkCC(coords);
                    if(c != nullptr) c->facesVisited.set((Chunk::Face)i);
                    //update square
                    if(USE_CPU_VISIBILITY) {
                        Square& s = squares[x & WORLDSIZE_MASK][y & WORLDSIZE_MASK][z & WORLDSIZE_MASK];
                        s = squareUnion(s, getSquare(coords, (Chunk::Face)i, viewMatrix));
                    }
                    //push it
                    q.push(std::make_pair(len, coords));
                }
    }
    Debugger::popMark(); //Initial BFS Nodes
    //bfs
    Debugger::pushMark("BFS Main Loop", "");
    while(!q.empty()) {
        std::pair<float, vec3i> currentP = q.top();
        vec3i current = currentP.second;
        q.pop();
        //Process current chunk
        if(visited[current.x & WORLDSIZE_MASK][current.y & WORLDSIZE_MASK][current.z & WORLDSIZE_MASK]) continue;
        visited[current.x & WORLDSIZE_MASK][current.y & WORLDSIZE_MASK][current.z & WORLDSIZE_MASK] = true;
        const Square& currentSquare = squares[current.x & WORLDSIZE_MASK][current.y & WORLDSIZE_MASK][current.z & WORLDSIZE_MASK];
        Chunk* currentChunk = getChunkCC(current); //used later inside neighbor loop
        if(USE_CPU_VISIBILITY && currentSquare.d == vec2f(0.0f)) {
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
            if(!insideCoords(minLoadedCoords, maxLoadedCoords, neighbor)) continue;
            //visited culling
            if(visited[neighbor.x & WORLDSIZE_MASK][neighbor.y & WORLDSIZE_MASK][neighbor.z & WORLDSIZE_MASK]) continue;
            //visibility culling
            if(currentChunk != nullptr && !currentChunk->visibilityTest((Chunk::Face)i)) continue;
            //fustrum culling
            colliderSphere.center = neighbor*CHUNKSIZE+colliderOffset;
            if(!Collision::intersects(cam->getFrustum(), colliderSphere, ommitedPlanes)) continue;
            //mark face as visited
            Chunk* neighborChunk = getChunkCC(neighbor);
            if(neighborChunk != nullptr) neighborChunk->facesVisited.set(Chunk::getOppositeFace((Chunk::Face)i));
            //update neighbor cone
            if(USE_CPU_VISIBILITY) {
                Debugger::pushMark("Square Calculation", "Time spent recalculating visibility squares");
                Square& ns = squares[neighbor.x & WORLDSIZE_MASK][neighbor.y & WORLDSIZE_MASK][neighbor.z & WORLDSIZE_MASK];
                ns = squareUnion(ns, squareIntersection(getSquare(current, (Chunk::Face)i, viewMatrix), currentSquare));
                Debugger::popMark(); //Square Calculation
            }
            //push it
            float len = glm::dot(vec3f(neighbor), sunDir);
            q.push(std::make_pair(len, neighbor));
        }
    }
    Debugger::popMark(); //BFS main loop
    Debugger::popMark(); //BFS time
    Debugger::numChunksSkippedShadow = skippedChunks;
    return chunksToDraw;
}

unsigned int World::sendDrawCommands(const std::list<Chunk*>& chunksToDraw, const ShaderProgram& program) const {
    Debugger::pushMark("Batching", "Time spent actually sending chunk geometry to the GPU");
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
    Debugger::popMark(); //batching
    return numDrawn;
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
