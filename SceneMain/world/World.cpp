#include "World.hpp"
#include "Column.hpp"
#include "Chunk.hpp"
#include "../DeferredContainer.hpp"
#include "Sun.hpp"

World::World() : generator(rand()), renderer(nullptr) {
	renderer = (DeferredContainer*)getGame()->getObjectByName("deferred");
	setName("world");
	for(int x = 0; x < WORLDSIZE; ++x)
		for(int z = 0; z < WORLDSIZE; ++z)
			columns[x][z] = nullptr;
	Sun* sun = new Sun();
	sun->addTo(this);
}

World::~World() {
}

void World::update(float deltaTime) {
	generator.discardTasks();
	Column* newCol = nullptr;
	while((newCol = generator.pullDone()) != nullptr) {
		if(columns[newCol->getX()&WORLDSIZE_MASK][newCol->getZ()&WORLDSIZE_MASK] != nullptr) delete newCol;
		else columns[newCol->getX()&WORLDSIZE_MASK][newCol->getZ()&WORLDSIZE_MASK] = newCol;
	}
	Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");
	vec2f playerChunkPos = vec2f(vec2i(cam->getWorldPos().x,cam->getWorldPos().z)/CHUNKSIZE);
	std::vector<std::pair<float,std::pair<int,int> > > tasks;
	for(int x = -WORLDSIZE/2; x < WORLDSIZE/2; ++x)
		for(int z = -WORLDSIZE/2; z < WORLDSIZE/2; ++z) {
			vec2f colPos = playerChunkPos + vec2f(x,z);
			Column* actual = getColumn(colPos.x*CHUNKSIZE,0,colPos.y*CHUNKSIZE);
			if((actual == nullptr || actual->getX() != colPos.x || actual->getZ() != colPos.y) && !generator.currentlyWorking(vec2i(colPos))) {
				if(actual != nullptr) delete actual;
				tasks.push_back(std::pair<float,std::pair<int,int> >(glm::length(playerChunkPos-colPos),std::pair<int,int>(colPos.x,colPos.y)));
				columns[int(colPos.x)&WORLDSIZE_MASK][int(colPos.y)&WORLDSIZE_MASK] = nullptr;
				continue;
			}
			if(actual == nullptr) continue;
			for(unsigned int y = 0; y < actual->getChunks().size(); ++y){
				Chunk* c = actual->getChunks()[y];
				if(c == nullptr) continue;
				c->update(deltaTime);
			}
		}
	std::sort(tasks.begin(),tasks.end());
	for(unsigned int i = 0; i < tasks.size(); ++i) generator.enqueueTask(vec2i(tasks[i].second.first,tasks[i].second.second));
}

void World::draw() const{
	Camera* cam;
	if(renderer->getMode() == DeferredContainer::Deferred)
		cam = (Camera*)getGame()->getObjectByName("playerCam");
	else if(renderer->getMode() == DeferredContainer::ShadowMap) {
		drawShadowMaps();
		return;
	}
	else return;

	std::priority_queue<std::pair<float,Chunk*> > queryList; //chunks to be queried, ordered by distance

	for(int x = 0; x < WORLDSIZE; ++x)
		for(int z = 0; z < WORLDSIZE; ++z) {
			Column* col = columns[x][z];
			if(col == nullptr) continue;
			for(unsigned int y = 0; y < col->getChunks().size(); ++y) {
				Chunk* actual = col->getChunks()[y];
				if(actual == nullptr || actual->isHidden() || !Collision::intersects(cam->getFrustum(),actual->getWorldSpaceBoundingBox())) continue;
				queryList.push(std::pair<float,Chunk*>(-glm::length(vec3f(actual->getAbsolutePos()) + vec3f(CHUNKSIZE/2)-cam->getWorldPos()),actual));
			}
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
		GL_ASSERT(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
		GL_ASSERT(glDepthMask(GL_FALSE));

		//generate and send the queries
		int queriesSent = 0;
		GL_ASSERT(glGenQueries(chunksPerLayer, &queries[0]));
		for (int i = 0; i < chunksPerLayer && queryList.size() > 0; ++i) {
			Chunk* currChunk = queryList.top().second;
			chunkPointers[i] = currChunk;
			queryList.pop();

			GL_ASSERT(glBeginQuery(GL_ANY_SAMPLES_PASSED,queries[i]));
			currChunk->drawBoundingBox();
			GL_ASSERT(glEndQuery(GL_ANY_SAMPLES_PASSED));
			++queriesSent;
		}

		//enable rendering state
		GL_ASSERT(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
		GL_ASSERT(glDepthMask(GL_TRUE));

		//collect query results
		for (int i = 0; i < queriesSent; ++i) {
			//if we have pending query, get result
			GLint seen;
			GL_ASSERT(glGetQueryObjectiv(queries[i],GL_QUERY_RESULT, &seen));
			//if seen, draw it
			if (seen > 0)
				chunkPointers[i]->draw();
		}
		//delete the queries
		GL_ASSERT(glDeleteQueries(queries.size(),&queries[0]));
	}
}

void World::drawShadowMaps() const {
	Sun* sun = (Sun*)getGame()->getObjectByName("sun");
	AABB occludersBox;
	AABB occludedBox;
	Camera* playerCam = (Camera*)getGame()->getObjectByName("playerCam");
	for(int x = 0; x < WORLDSIZE; ++x)
		for(int z = 0; z < WORLDSIZE; ++z) {
			Column* col = columns[x][z];
			if(col == nullptr) continue;
			for(unsigned int y = 0; y < col->getChunks().size(); ++y) {
				Chunk* actual = col->getChunks()[y];
				if(actual == nullptr) continue;
				occludersBox.extend(actual->getWorldSpaceBoundingBox());
				if(actual->isHidden() || !Collision::intersects(playerCam->getFrustum(), actual->getWorldSpaceBoundingBox())) continue;
				occludedBox.extend(actual->getWorldSpaceBoundingBox());
			}
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

unsigned int World::getCube(int x, int y, int z) const {
	Column* c = getColumn(x,y,z);
	return (c == nullptr)? 0 : c->getCube(x & CHUNKSIZE_MASK,y,z & CHUNKSIZE_MASK);
}

void World::setCube(int x, int y, int z, unsigned int cube) {
	Column* c = getColumn(x,y,z);
	if(c == nullptr) return;
	c->setCube(x & CHUNKSIZE_MASK, y, z & CHUNKSIZE_MASK, cube);
}
