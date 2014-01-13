#include "World.hpp"
#include "Column.hpp"
#include "Chunk.hpp"
#include "../DeferredContainer.hpp"
#include "../Camera.hpp"

World::World() : generator(rand()), renderer(nullptr) {
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
			for(unsigned int y = 0; y < columns[x][z]->getChunks().size(); ++y){
				Chunk* actual = columns[x][z]->getChunks()[y];
				if(actual == nullptr) continue;
				actual->update(deltaTime);
			}
}

void World::draw() const{
	if(renderer->getMode() != DeferredContainer::Deferred) return;
	Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");

	std::priority_queue<std::pair<float,Chunk*> > queryList; //chunks to be queried, ordered by distance

		for(unsigned int x = 0; x < WORLDSIZE; ++x)
			for(unsigned int z = 0; z < WORLDSIZE; ++z)
				for(unsigned int y = 0; y < columns[x][z]->getChunks().size(); ++y) {
					Chunk* actual = columns[x][z]->getChunks()[y];
				if(actual == nullptr || actual->getVertexCount() == 0 || !cam->getFrustum().insideFrustum(vec3f(actual->getAbsolutePos()+vec3i(CHUNKSIZE/2)),sqrt(3)*CHUNKSIZE)) continue;
				queryList.push(std::pair<float,Chunk*>(-glm::length(vec3f(actual->getAbsolutePos()) + vec3f(CHUNKSIZE/2)-cam->getWorldPos()),actual));
			}

	//do occlusion culling here!
	int layers = 10;
	int chunksPerLayer = queryList.size()/layers + int(queryList.size()%layers > 0); //chunks per pass
	//first layer is always drawn
	for(int i = 0; i < chunksPerLayer && queryList.size() > 0; i++) {
		std::pair<float,Chunk*> c = queryList.top();
		queryList.pop();
		c.second->draw();
	}
	//Query other layers
	for(int currLayer = 1; currLayer < layers && queryList.size() > 0; ++currLayer) {
		std::vector<GLuint> queries(chunksPerLayer,0);
		std::vector<Chunk*> chunkPointers(chunksPerLayer,nullptr);

		//disable rendering state
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);

		//generate and send the queries
		int queriesSent = 0;
		glGenQueries(chunksPerLayer, &queries[0]);
		for (int i = 0; i < chunksPerLayer && queryList.size() > 0; ++i) {
			Chunk* currChunk = queryList.top().second;
			chunkPointers[i] = currChunk;
			queryList.pop();

			glBeginQuery(GL_ANY_SAMPLES_PASSED,queries[i]);
			currChunk->drawBoundingBox();
			glEndQuery(GL_ANY_SAMPLES_PASSED);
			++queriesSent;
		}

		//enable rendering state
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);

		//collect query results
		for (int i = 0; i < queriesSent; ++i) {
			//if we have pending query, get result
			GLint seen;
			glGetQueryObjectiv(queries[i],GL_QUERY_RESULT, &seen);
			//if seen, draw it
			if (seen > 0)
				chunkPointers[i]->draw();
		}
		//delete the queries
		glDeleteQueries(queries.size(),&queries[0]);
	}
}

bool World::outOfBounds(int x, int y, int z) const {
	Column* c = columns[(x >> CHUNKSIZE_POW2) & WORLDSIZE_MASK][(z >> CHUNKSIZE_POW2) & WORLDSIZE_MASK];
	return (c == nullptr || c->getX() != (x >> CHUNKSIZE_POW2) || c->getZ() != (z >> CHUNKSIZE_POW2) || y < 0);
}

Column* World::getColumn(int x, int y, int z) const {
	Column* c = columns[(x >> CHUNKSIZE_POW2) & WORLDSIZE_MASK][(z >> CHUNKSIZE_POW2) & WORLDSIZE_MASK];
	return (c == nullptr || c->getX() != (x >> CHUNKSIZE_POW2) || c->getZ() != (z >> CHUNKSIZE_POW2) || y < 0)? nullptr:c;
}

Camera* World::getCamera() const {
	return (Camera*)getGame()->getObjectByName("playerCam");
}

Cube World::getCube(int x, int y, int z) const {
	Column* c = getColumn(x,y,z);
	return (c == nullptr)? Cube(0,MINLIGHT):c->getCube(x & CHUNKSIZE_MASK,y,z & CHUNKSIZE_MASK);
}

void World::setCubeID(int x, int y, int z, unsigned char ID) {
	Column* c = getColumn(x,y,z);
	if(c == nullptr) return;
	c->setCubeID(x & CHUNKSIZE_MASK, y, z & CHUNKSIZE_MASK, ID);
}

void World::setCubeLight(int x, int y, int z, unsigned char light) {
	Column* c = getColumn(x,y,z);
	if(c == nullptr) return;
	c->setCubeLight(x & CHUNKSIZE_MASK, y, z & CHUNKSIZE_MASK, light);
}
