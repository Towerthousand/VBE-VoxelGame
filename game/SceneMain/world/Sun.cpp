#include "Sun.hpp"
#include "World.hpp"
#include "Column.hpp"
#include "Chunk.hpp"

Sun::Sun() : angle(45.0f) {
	setName("sun");
	minZ = std::vector<float>(NUM_SUN_CASCADES, 0.0f);
	maxZ = std::vector<float>(NUM_SUN_CASCADES, 0.0f);
	float zFar = std::min(WORLDSIZE*CHUNKSIZE, 256);
	float numParts = (1 << (NUM_SUN_CASCADES)) - 1;
	float lastMax = 0.0f;
	for(int i = 0; i < NUM_SUN_CASCADES; ++i) {
		cameras[i] = new Camera();
		cameras[i]->projection = glm::ortho(-10,10,-10,10,-100,100);
		cameras[i]->addTo(this);
		minZ[i] = lastMax;
		maxZ[i] = zFar*((1 << i)/numParts);
		lastMax = maxZ[i];
	}
	cameras[NUM_SUN_CASCADES] = new Camera();
	cameras[NUM_SUN_CASCADES]->projection = glm::ortho(-10,10,-10,10,-100,100);
	cameras[NUM_SUN_CASCADES]->addTo(this);
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
	recalculateAABBs();
	for(unsigned int i = 0; i < NUM_SUN_CASCADES+1; ++i) {
		cameras[i]->pos = aabbs[i].getCenter();
		cameras[i]->lookInDir(getDirection());
		min[i] = vec3f(std::numeric_limits<float>::max());
		max[i] = vec3f(std::numeric_limits<float>::lowest());
		extend({i}, aabbs[i]);
	}
	for(int i = 0; i < NUM_SUN_CASCADES+1; ++i) {
		cameras[i]->pos -= cameras[i]->getForward()*(glm::abs(min[i].z));
		max[i].z += glm::abs(min[i].z);
		cameras[i]->projection = glm::ortho(min[i].x, max[i].x, min[i].y, max[i].y, min[i].z, max[i].z);
		cameras[i]->recalculateFrustum();
	}
	for(int i = 0; i < NUM_SUN_CASCADES; ++i) {
		VP[i] = cameras[i]->projection*cameras[i]->getView();
	}
}

void Sun::recalculateCameras() {
	for(int i = 0; i < NUM_SUN_CASCADES+1; ++i) {
		cameras[i]->pos = aabbs[i].getCenter();
		cameras[i]->lookInDir(getDirection());
		min[i] = vec3f(std::numeric_limits<float>::max());
		max[i] = vec3f(std::numeric_limits<float>::lowest());
	}
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
					for(int i = 0; i < NUM_SUN_CASCADES; ++i) {
						float dist = glm::abs(vec3f(pCam->getView() * vec4f(actual->getWorldSpaceBoundingBox().getCenter(), 1.0f)).z);
						if(dist < maxZ[i]+CHUNKSIZE*1.73 && dist > minZ[i]-CHUNKSIZE*1.73)
							indexes.push_back(i);
					}
					if(!indexes.empty()) {
						indexes.push_back(NUM_SUN_CASCADES);
						extend(indexes, actual->getWorldSpaceBoundingBox());
					}
				}
			}
		}
}

void Sun::recalculateAABBs() {
	for(int i = 0; i < NUM_SUN_CASCADES+1; ++i) aabbs[i] = AABB();
	Camera* pCam = (Camera*)getGame()->getObjectByName("playerCam");
	World* w = (World*)getGame()->getObjectByName("world");
	for(int x = 0; x < WORLDSIZE; ++x)
		for(int z = 0; z < WORLDSIZE; ++z) {
			Column* col = w->columns[x][z];
			if(col == nullptr) continue;
			for(unsigned int y = 0; y < col->getChunkCount(); ++y) {
				Chunk* actual = col->getChunkCC(y);
				if(actual != nullptr && actual->wasDrawedByPlayer()) {
					bool include = false;
					for(int i = 0; i < NUM_SUN_CASCADES; ++i) {
						float dist = glm::abs(vec3f(pCam->getView() * vec4f(actual->getWorldSpaceBoundingBox().getCenter(), 1.0f)).z);
						if(dist < maxZ[i]+CHUNKSIZE*1.73 && dist > minZ[i]-CHUNKSIZE*1.73) {
							aabbs[i].extend(actual->getWorldSpaceBoundingBox());
							numOccluders[i]++;
							include = true;
						}
					}
					if(include) {
						aabbs[NUM_SUN_CASCADES].extend(actual->getWorldSpaceBoundingBox());
						numOccluders[NUM_SUN_CASCADES]++;
					}
				}
			}
		}
}

void Sun::extend(std::vector<unsigned int> index, const AABB& occludedBox) {
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
		for(unsigned int j : index) {
			vec3f p = projections[i];
			p = vec3f(cameras[j]->getView()*vec4f(p, 1.0f)); //project this corner onto the
			max[j].x = std::max(max[j].x, p.x);
			min[j].x = std::min(min[j].x, p.x);
			max[j].y = std::max(max[j].y, p.y);
			min[j].y = std::min(min[j].y, p.y);
			max[j].z = std::max(max[j].z, p.z);
			min[j].z = std::min(min[j].z, p.z);
		}
	}
}
