#include "Sun.hpp"
#include "World.hpp"
#include "Column.hpp"
#include "Chunk.hpp"
#include "../Player.hpp"

Sun::Sun() : angle(45.0f) {
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
	maxZ[0] = 8.0f;
	minZ[1] = 8.0f;
	maxZ[1] = 32.0f;
	minZ[2] = 32.0f;
	maxZ[2] = 128.0f;
	minZ[3] = 128.0f;
	maxZ[3] = 256.0f;
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
	Player* player = (Player*)this->getGame()->getObjectByName("player");
	for(int i = 0; i < NUM_SUN_CASCADES+1; ++i) {
		cameras[i]->pos = player->getPosition();
		cameras[i]->lookInDir(getDirection());
		cameras[i]->projection = glm::ortho(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		aabbs[i] = AABB(); //sun view space bounding box for the player-drawn geometry
	}
	//extend the view-space bounding box to enclose the user-seen chunks
	AABB worldAABB = extendFrustums();
	for(int i = 0; i < NUM_SUN_CASCADES+1; ++i) {
		//unfold the view space AABB into worldspace and position the camera in it's center
		//first, take the world space center of the view-space bounding box
		vec3f worldCenter = vec3f(glm::inverse(cameras[i]->getView())*vec4f(aabbs[i].getCenter(), 1.0f));
		cameras[i]->pos = worldCenter;
		aabbs[i] = AABB(aabbs[i].getMin() - aabbs[i].getCenter(), aabbs[i].getMax() - aabbs[i].getCenter());
		float dist = getDistanceFromOutside(i, worldAABB);
		//take the camera to the back of the box, so the bfs executes correctly on World::draw();
		cameras[i]->pos -= cameras[i]->getForward()*dist;
		//adjust minz and maxz accordingly
		aabbs[i] = AABB(aabbs[i].getMin() + vec3f(0.0f,0.0f, dist), aabbs[i].getMax() + vec3f(0.0f, 0.0f, dist));
		//build actual frustum
		cameras[i]->projection = glm::ortho(aabbs[i].getMin().x, aabbs[i].getMax().x, aabbs[i].getMin().y, aabbs[i].getMax().y, aabbs[i].getMin().z, aabbs[i].getMax().z);
		cameras[i]->recalculateFrustum();
	}
	for(int i = 0; i < NUM_SUN_CASCADES; ++i)
		VP[i] = cameras[i]->projection*cameras[i]->getView();
}

AABB Sun::extendFrustums() {
	AABB worldAABB = AABB();
	Camera* pCam = (Camera*)getGame()->getObjectByName("playerCam");
	World* w = (World*)getGame()->getObjectByName("world");
	for(int x = 0; x < WORLDSIZE; ++x)
		for(int z = 0; z < WORLDSIZE; ++z) {
			Column* col = w->columns[x][z];
			if(col == nullptr) continue;
			for(unsigned int y = 0; y < col->getChunkCount(); ++y) {
				Chunk* actual = col->getChunkCC(y);
				if(actual == nullptr) continue;
				if(actual->hasMesh()) worldAABB.extend(actual->getWorldSpaceBoundingBox());
				if(actual->wasDrawedByPlayer()) {
					std::vector<unsigned int> indexes;
					AABB bbox = actual->getWorldSpaceBoundingBox();
					for(int i = 0; i < NUM_SUN_CASCADES; ++i) {
						float dist = glm::abs(vec3f(pCam->getView() * vec4f(bbox.getCenter(), 1.0f)).z);
						if(dist < maxZ[i]+bbox.getRadius() && dist > minZ[i]-bbox.getRadius())
							indexes.push_back(i);
					}
					if(!indexes.empty()) {
						indexes.push_back(NUM_SUN_CASCADES);
						extend(indexes, actual->getWorldSpaceBoundingBox());
					}
				}
			}
		}
	return worldAABB;
}

void Sun::extend(std::vector<unsigned int> index, const AABB& occludedBox) {
	for(int i = 0; i < 2; ++i)
		for(int j = 0; j < 2; ++j)
			for(int k = 0; k < 2; ++k) {
				//project this corner onto the view plane. view matrix is the same
				//for all cameras at this point so we'll just multiply once
				vec3f p = vec3f(cameras[0]->getView()
								* vec4f(occludedBox.getMin() + occludedBox.getDimensions()*vec3f(i, j, k), 1.0f));
				for(unsigned int j : index)	aabbs[j].extend(p);
			}
}

float Sun::getDistanceFromOutside(unsigned int index, const AABB& worldAABB) {
	float dist = 0.0f;
	for(int i = 0; i < 2; ++i)
		for(int j = 0; j < 2; ++j)
			for(int k = 0; k < 2; ++k) {
				vec3f p = vec3f(cameras[index]->getView()
								* vec4f(worldAABB.getMin() + worldAABB.getDimensions()*vec3f(i, j, k), 1.0f));
				if(p.z < dist) dist = p.z;
			}
	return glm::abs(dist);
}
