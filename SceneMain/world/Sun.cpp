#include "Sun.hpp"
#include "World.hpp"
#include "Column.hpp"
#include "Chunk.hpp"

Sun::Sun() : angle(45.0f) {
	setName("sun");
	cam = new Camera("sunCamera");
	cam->projection = glm::ortho(-10,10,-10,10,-100,100);
	cam->addTo(this);
}

Sun::~Sun() {
}

void Sun::update(float deltaTime) {
	(void) deltaTime;
	if(Environment::getKeyboard()->isKeyHeld(Keyboard::Z)) angle -= 1.0f*deltaTime;
	if(Environment::getKeyboard()->isKeyHeld(Keyboard::X)) angle += 1.0f*deltaTime;
}

void Sun::updateCamera() {
	AABB occludedBox;
	Camera* pCam = (Camera*)getGame()->getObjectByName("playerCam");
	World* w = (World*)getGame()->getObjectByName("world");
	for(int x = 0; x < WORLDSIZE; ++x)
		for(int z = 0; z < WORLDSIZE; ++z) {
			Column* col = w->columns[x][z];
			if(col == nullptr) continue;
			for(unsigned int y = 0; y < col->getChunkCount(); ++y) {
				Chunk* actual = col->getChunkCC(y);
				if(actual != nullptr && actual->wasDrawedByPlayer() && glm::length(actual->getWorldSpaceBoundingBox().getCenter()-pCam->getWorldPos()) < 8*1.8) {
					Log::message() << actual->getWorldSpaceBoundingBox().getMin() << "YAY" << actual->getWorldSpaceBoundingBox().getMin() << Log::Flush;
					occludedBox.extend(actual->getWorldSpaceBoundingBox());
				}
			}
		}

	Log::message() << occludedBox.getMin() << " JODER " <<  occludedBox.getMax() << " MIERDA " << occludedBox.getDimensions() << Log::Flush;
	cam->pos = occludedBox.getCenter();
	cam->lookInDir(getDirection());

	vec3f min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::min());
	std::vector<vec3f> projections(8);
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
		p = vec3f(cam->getView()*vec4f(p, 1.0f));
		max.x = (max.x < p.x)? p.x : max.x;
		min.x = (min.x > p.x)? p.x : min.x;
		max.y = (max.y < p.y)? p.y : max.y;
		min.y = (min.y > p.y)? p.y : min.y;
		max.z = (max.z < p.z)? p.z : max.z;
		min.z = (min.z > p.z)? p.z : min.z;
	}
	cam->pos -= cam->getForward()*(glm::abs(min.z));
	max.z += glm::abs(min.z);
	cam->projection = glm::ortho(min.x, max.x, min.y, max.y, min.z, max.z);
	cam->recalculateFrustum();
}
