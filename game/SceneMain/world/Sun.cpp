#include "Sun.hpp"
#include "World.hpp"
#include "Column.hpp"
#include "Chunk.hpp"
#include "../Player.hpp"

Sun::Sun() {
    setName("sun");
    minZ = std::vector<float>(NUM_SUN_CASCADES, 0.0f);
    maxZ = std::vector<float>(NUM_SUN_CASCADES, 0.0f);
    float zFar = std::min(WORLDSIZE*CHUNKSIZE, 256);
    float numParts = (1 << (NUM_SUN_CASCADES)) - 1;
    float lastMax = 0.0f;
    for(int i = 0; i < NUM_SUN_CASCADES; ++i) {
        cameras[i] = new Camera();
        cameras[i]->addTo(this);
        minZ[i] = lastMax;
        maxZ[i] = zFar*(float(1 << i)/float(numParts));
        lastMax = maxZ[i];
    }
    cameras[NUM_SUN_CASCADES] = new Camera();
    cameras[NUM_SUN_CASCADES]->projection = glm::ortho(-10,10,-10,10,-100,100);
    cameras[NUM_SUN_CASCADES]->addTo(this);
    //TODO: figure out best partitioning instead of hard-coding this
    minZ[0] = -10000;
    maxZ[0] = 8.0f;
    minZ[1] = 8.0f;
    maxZ[1] = 32.0f;
    minZ[2] = 32.0f;
    maxZ[2] = 128.0f;
    minZ[3] = 128.0f;
    maxZ[3] = 256.0f;
    VP = std::vector<mat4f>(NUM_SUN_CASCADES, mat4f(1.0f));
}

Sun::~Sun() {
}

void Sun::update(float deltaTime) {
    (void) deltaTime;
    if(Keyboard::pressed(Keyboard::Z)) angle -= 1.0f*deltaTime;
    if(Keyboard::pressed(Keyboard::X)) angle += 1.0f*deltaTime;
}

void Sun::updateCameras() {
    Camera* playerCam = (Camera*)this->getGame()->getObjectByName("playerCam");
    for(int i = 0; i < NUM_SUN_CASCADES+1; ++i) {
        cameras[i]->pos = vec3f(0.0f);
        cameras[i]->lookInDir(getDirection());
        cameras[i]->projection = glm::ortho(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        aabbs[i] = AABB(); //sun view space bounding box for the player-drawn geometry
    }
    //extend the view-space bounding box to enclose the user-seen chunks
    extendFrustums();
    for(int i = 0; i < NUM_SUN_CASCADES+1; ++i) {
        //transform from player view coords to sun cam view coords
        AABB trueBB = AABB();
        for(int i2 = 0; i2 < 2; ++i2)
            for(int j = 0; j < 2; ++j)
                for(int k = 0; k < 2; ++k) {
                    vec3f p = aabbs[i].getMin() + aabbs[i].getDimensions()*vec3f(i2, j, k);
                    if(false && i != NUM_SUN_CASCADES)
                        p = {p.x, p.y, glm::min(glm::max(minZ[i], p.z), maxZ[i])};
                    p = vec3f(glm::inverse(playerCam->getView()) * vec4f(p, 1.0));
                    p = vec3f(cameras[i]->getView()*vec4f(p, 1.0));
                    trueBB.extend(p);
                }
        aabbs[i] = trueBB;
        //unfold the view space AABB into worldspace and position the camera in it's center
        vec3f worldCenter = vec3f(glm::inverse(cameras[i]->getView())*vec4f(aabbs[i].getCenter(), 1.0f));
        cameras[i]->pos = worldCenter;
        aabbs[i] = AABB(aabbs[i].getMin() - aabbs[i].getCenter(), aabbs[i].getMax() - aabbs[i].getCenter());
        //build actual frustum
        cameras[i]->projection = glm::ortho(aabbs[i].getMin().x, aabbs[i].getMax().x, aabbs[i].getMin().y, aabbs[i].getMax().y, aabbs[i].getMin().z, aabbs[i].getMax().z);
        cameras[i]->recalculateFrustum();
    }
    for(int i = 0; i < NUM_SUN_CASCADES; ++i)
        VP[i] = cameras[i]->projection*cameras[i]->getView();
}

void Sun::extendFrustums() {
    Camera* pCam = (Camera*)getGame()->getObjectByName("playerCam");
    World* w = (World*)getGame()->getObjectByName("world");
    for(int x = 0; x < WORLDSIZE; ++x)
        for(int z = 0; z < WORLDSIZE; ++z) {
            Column* col = w->columns[x][z];
            if(col == nullptr) continue;
            for(unsigned int y = 0; y < col->getChunkCount(); ++y) {
                Chunk* actual = col->getChunkCC(y);
                if(actual != nullptr && actual->wasDrawedByPlayer()) {
                    std::vector<unsigned int> indexes;
                    AABB bbox = actual->getWorldSpaceBoundingBox();
                    for(int i = 0; i < NUM_SUN_CASCADES; ++i) {
                        float dist = glm::abs(vec3f(pCam->getView() * vec4f(bbox.getCenter(), 1.0f)).z);
                        if(dist < maxZ[i]+bbox.getRadius() && dist > minZ[i]-bbox.getRadius())
                            indexes.push_back(i);
                    }
                    if(!indexes.empty()) {
                        indexes.push_back(NUM_SUN_CASCADES);
                        extend(indexes, actual->getWorldSpaceBoundingBox(), pCam);
                    }
                }
            }
        }
}

void Sun::extend(std::vector<unsigned int> index, const AABB& occludedBox, const Camera* pCam) {
    for(int i = 0; i < 2; ++i)
        for(int j = 0; j < 2; ++j)
            for(int k = 0; k < 2; ++k) {
                //project this corner onto the view plane. view matrix is the same
                //for all cameras at this point so we'll just multiply once
                vec3f p = vec3f(
                    pCam->getView() *
                    vec4f(
                        occludedBox.getMin() + occludedBox.getDimensions()*vec3f(i, j, k),
                        1.0f
                    )
                );
                for(unsigned int j : index) aabbs[j].extend(p);
            }
}
