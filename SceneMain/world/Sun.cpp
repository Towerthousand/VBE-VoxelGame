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
	AABB occludersBox;
	AABB occludedBox;
	World* w = (World*)getGame()->getObjectByName("world");
	for(int x = 0; x < WORLDSIZE; ++x)
		for(int z = 0; z < WORLDSIZE; ++z) {
			Column* col = w->columns[x][z];
			if(col == nullptr) continue;
			for(unsigned int y = 0; y < col->getChunkCount(); ++y) {
				Chunk* actual = col->getChunkCC(y);
				if(actual != nullptr && actual->hasMesh()) {
					occludersBox.extend(actual->getWorldSpaceBoundingBox());
					if(actual->wasDrawedByPlayer())
						occludedBox.extend(actual->getWorldSpaceBoundingBox());
				}
			}
		}
	cam->pos = occludedBox.getCenter();
	cam->lookInDir(getDirection());

	vec3f min(0.0f), max(0.0f);
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
		max.x = std::max(max.x, p.x);
		max.y = (max.y < p.y)? p.y : max.y;
		min.x = (min.x > p.x)? p.x : min.x;
		min.y = (min.y > p.y)? p.y : min.y;
		max.z = (max.z < p.z)? p.z : max.z;
	}
	std::vector<vec3f> projections2(8);
	projections2[0] = occludersBox.getMin();
	projections2[1] = occludersBox.getMin() + occludersBox.getDimensions()*vec3f(0, 0, 1);
	projections2[2] = occludersBox.getMin() + occludersBox.getDimensions()*vec3f(0, 1, 0);
	projections2[3] = occludersBox.getMin() + occludersBox.getDimensions()*vec3f(0, 1, 1);
	projections2[4] = occludersBox.getMin() + occludersBox.getDimensions()*vec3f(1, 0, 0);
	projections2[5] = occludersBox.getMin() + occludersBox.getDimensions()*vec3f(1, 0, 1);
	projections2[6] = occludersBox.getMin() + occludersBox.getDimensions()*vec3f(1, 1, 0);
	projections2[7] = occludersBox.getMin() + occludersBox.getDimensions();
	for(int i = 0; i < 8; ++i) {
		vec3f& p = projections2[i];
		p = vec3f(cam->getView()*vec4f(p, 1.0f));
		min.z = (min.z > p.z)? p.z : min.z;
	}
	min.z -= 1.75f*CHUNKSIZE;
	cam->pos -= cam->getForward()*(glm::abs(min.z));
	max.z += glm::abs(min.z);
	cam->projection = glm::ortho(min.x, max.x, min.y, max.y, min.z, max.z);
	cam->recalculateFrustum();
}
