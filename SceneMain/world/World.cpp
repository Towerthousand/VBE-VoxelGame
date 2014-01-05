#include "World.hpp"
#include "Column.hpp"
#include "Chunk.hpp"
#include "../DeferredContainer.hpp"
#include "../Camera.hpp"

World::World() : generator(1), renderer(nullptr) {
	renderer = (DeferredContainer*)getGame()->getObjectByName("deferred");
	setName("World");
	for(int x = 0; x < WORLDSIZE; ++x)
		for(int z = 0; z < WORLDSIZE; ++z)
			columns[x][z] = generator.getColumn(x,z);
}

World::~World() {
}

void World::update(float deltaTime) {
	for(unsigned int x = 0; x < WORLDSIZE; ++x)
		for(unsigned int z = 0; z < WORLDSIZE; ++z)
			for(unsigned int y = 0; y < columns[x][z]->getChunks().size(); ++y)
				columns[x][z]->getChunks()[y]->update(deltaTime);
}

void World::draw() const{
	if(renderer->getMode() != DeferredContainer::Deferred) return;
	Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");
	for(unsigned int x = 0; x < WORLDSIZE; ++x)
		for(unsigned int z = 0; z < WORLDSIZE; ++z)
			for(unsigned int y = 0; y < columns[x][z]->getChunks().size(); ++y) {
				Chunk* actual = columns[x][z]->getChunks()[y];
				if(cam->getFrustum().insideFrustum(vec3f(actual->getAbsolutePos()+vec3i(CHUNKSIZE/2)),sqrt(3)*CHUNKSIZE))
					actual->draw();
			}
}

bool World::outOfBounds(int x, int y, int z) const {
	Column* c = columns[(x >> CHUNKSIZE_POW2) & WORLDSIZE_MASK][(z >> CHUNKSIZE_POW2) & WORLDSIZE_MASK];
	return (c == nullptr || c->getX() != (x >> CHUNKSIZE_POW2) || c->getZ() != (z >> CHUNKSIZE_POW2) || y < 0);
}

Column* World::getColumn(int x, int y, int z) const {
	if(outOfBounds(x,y,z)) return nullptr;
	return columns[(x >> CHUNKSIZE_POW2) & WORLDSIZE_MASK][(z >> CHUNKSIZE_POW2) & WORLDSIZE_MASK];
}

Camera* World::getCamera() const {
	return (Camera*)getGame()->getObjectByName("playerCam");
}

Cube World::getCube(int x, int y, int z) const {
	Column* c = getColumn(x,y,z);
	if(c == nullptr) return Cube(0,MINLIGHT);
	return c->getCube(x & CHUNKSIZE_MASK,y,z & CHUNKSIZE_MASK);
}

void World::setCubeID(int x, unsigned int y, int z, unsigned char ID) {

}

void World::setCubeLight(int x, unsigned int y, int z, unsigned char light) {

}
