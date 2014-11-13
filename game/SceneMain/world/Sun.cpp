#include "Sun.hpp"
#include "World.hpp"
#include "Column.hpp"
#include "Chunk.hpp"

Sun::Sun() : angle(45.0f) {
	setName("sun");
	maxZ = std::vector<float>(NUM_SUN_CASCADES, 0.0f);
	float zFar = WORLDSIZE*CHUNKSIZE*0.25f;
	float numParts = pow(2, NUM_SUN_CASCADES) - 1;
	float lastMax = 0.0f;
	globalCam = new Camera();
	globalCam->projection = glm::ortho(-10,10,-10,10,-100,100);
	globalCam->addTo(this);
	for(int i = 0; i < NUM_SUN_CASCADES; ++i) {
		cameras[i] = new Camera();
		cameras[i]->projection = glm::ortho(-10,10,-10,10,-100,100);
		cameras[i]->addTo(this);
		minZ[i] = lastMax;
		maxZ[i] = zFar*(pow(2,i)/numParts);
		lastMax = maxZ[i];
	}
	minZ[0] = -10000;
	maxZ[NUM_SUN_CASCADES-1] = zFar;
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
	AABB globalAABB = AABB();
	for(int i = 0; i < NUM_SUN_CASCADES; ++i) {
		numOccluders[i] = 0;
		calculateAABB(i);
		updateSingleCam(cameras[i], aabbs[i]);
		VP[i] = cameras[i]->projection*cameras[i]->getView();
		if(numOccluders[i] != 0) globalAABB.extend(aabbs[i]);
	}
	updateSingleCam(globalCam, globalAABB);
}

void Sun::calculateAABB(unsigned int camID) {
	aabbs[camID] = AABB();
	Camera* pCam = (Camera*)getGame()->getObjectByName("playerCam");
	World* w = (World*)getGame()->getObjectByName("world");
	for(int x = 0; x < WORLDSIZE; ++x)
		for(int z = 0; z < WORLDSIZE; ++z) {
			Column* col = w->columns[x][z];
			if(col == nullptr) continue;
			for(unsigned int y = 0; y < col->getChunkCount(); ++y) {
				Chunk* actual = col->getChunkCC(y);
				if(actual != nullptr && actual->wasDrawedByPlayer()) {
					float dist = glm::length(vec3f(pCam->getView() * vec4f(actual->getWorldSpaceBoundingBox().getCenter(), 1.0f)));
					if(dist < maxZ[camID]+CHUNKSIZE*1.73 && dist > minZ[camID]-CHUNKSIZE*1.73) {
						aabbs[camID].extend(actual->getWorldSpaceBoundingBox());
						numOccluders[camID]++;
					}
				}
			}
		}
}
void Sun::updateSingleCam(Camera* cam, const AABB& occludedBox) {
	cam->pos = occludedBox.getCenter();
	cam->lookInDir(getDirection());

	vec3f min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::min());
	vec3f projections[8];
	projections[0] = occludedBox.getMin();
	projections[1] = occludedBox.getMin() + occludedBox.getDimensions()*vec3f(0, 0, 1);
	projections[2] = occludedBox.getMin() + occludedBox.getDimensions()*vec3f(0, 1, 0);
	projections[3] = occludedBox.getMin() + occludedBox.getDimensions()*vec3f(0, 1, 1);
	projections[4] = occludedBox.getMin() + occludedBox.getDimensions()*vec3f(1, 0, 0);
	projections[5] = occludedBox.getMin() + occludedBox.getDimensions()*vec3f(1, 0, 1);
	projections[6] = occludedBox.getMin() + occludedBox.getDimensions()*vec3f(1, 1, 0);
	projections[7] = occludedBox.getMin() + occludedBox.getDimensions();
	for(int i = 0; i < 8; ++i) {
		vec3f& p = projections[i];
		p = vec3f(cam->getView()*vec4f(p, 1.0f)); //project this corner onto the
		max.x = std::max(max.x, p.x);
		min.x = std::min(min.x, p.x);
		max.y = std::max(max.y, p.y);
		min.y = std::min(min.y, p.y);
		max.z = std::max(max.z, p.z);
		min.z = std::min(min.z, p.z);
	}
	cam->pos -= cam->getForward()*(glm::abs(min.z));
	max.z += glm::abs(min.z);
	cam->projection = glm::ortho(min.x, max.x, min.y, max.y, min.z, max.z);
	cam->recalculateFrustum();
}
