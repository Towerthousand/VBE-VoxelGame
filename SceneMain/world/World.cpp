#include "World.hpp"
#include "Chunk.hpp"
#include "../DeferredContainer.hpp"
#include "Sun.hpp"
#include "Level.hpp"
#include "../NetworkManager.hpp"

World::World() : renderer(nullptr) {
	renderer = (DeferredContainer*)getGame()->getObjectByName("deferred");
	level = &((NetworkManager*)getGame()->getObjectByName("NetworkManager"))->level;
	setName("world");

	Sun* sun = new Sun();
	sun->addTo(this);

	chunks.resize(level->tx*level->ty*level->tz/CHUNKSIZE/CHUNKSIZE/CHUNKSIZE);
	for(int y = 0; y < level->ty/CHUNKSIZE; ++y)
		for(int z = 0; z < level->tz/CHUNKSIZE; ++z)
			for(int x = 0; x < level->tx/CHUNKSIZE; ++x)
				setChunk(x, y, z, new Chunk(x, y, z));
}

World::~World() {
}

void World::update(float deltaTime) {
	for(int y = 0; y < level->ty/CHUNKSIZE; ++y)
		for(int z = 0; z < level->tz/CHUNKSIZE; ++z)
			for(int x = 0; x < level->tx/CHUNKSIZE; ++x)
				getChunk(x, y, z)->update(deltaTime);

}

void World::draw() const {
	switch(renderer->getMode()) {
		case DeferredContainer::Deferred:
			draw((Camera*)getGame()->getObjectByName("playerCam"));
			break;
		case DeferredContainer::ShadowMap:
			draw((Camera*)getGame()->getObjectByName("sunCamera"));
			break;
		default: break;
	}
}

void World::draw(Camera* cam) const{
	std::priority_queue<std::pair<float,Chunk*> > queryList; //chunks to be queried, ordered by distance
	for(int y = 0; y < level->ty/CHUNKSIZE; ++y)
		for(int z = 0; z < level->tz/CHUNKSIZE; ++z)
			for(int x = 0; x < level->tx/CHUNKSIZE; ++x)
			{
				Chunk* actual = getChunk(x, y, z);
				if(actual == nullptr || actual->isHidden() || !Collision::intersects(cam->getFrustum(),actual->getWorldSpaceBoundingBox())) continue;
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

bool World::outOfBounds(int x, int y, int z) const {
	if(x < 0 || x >= level->tx) return true;
	if(y < 0 || y >= level->ty) return true;
	if(z < 0 || z >= level->tz) return true;
	return false;
}

Chunk* World::getChunk(int x, int y, int z) const
{
	int tx = level->tx/CHUNKSIZE;
	int ty = level->ty/CHUNKSIZE;
	int tz = level->tz/CHUNKSIZE;

	return x >= 0 && y >= 0 && z >= 0 && x < tx && y < ty && z < tz
			? chunks[(y * tz + z) * tx + x] : nullptr;
}

void World::setChunk(int x, int y, int z, Chunk* c)
{
	int tx = level->tx/CHUNKSIZE;
	int ty = level->ty/CHUNKSIZE;
	int tz = level->tz/CHUNKSIZE;

	if(x >= 0 && y >= 0 && z >= 0 && x < tx && y < ty && z < tz)
		chunks[(y * tz + z) * tx + x] = c;
}

Camera* World::getCamera() const {
	return (Camera*)getGame()->getObjectByName("playerCam");
}

unsigned char World::getBlock(int x, int y, int z) const {
	return level->getBlock(x, y, z);
}

const int dx[] = {1, -1, 0, 0, 0, 0};
const int dy[] = {0, 0, 1, -1, 0, 0};
const int dz[] = {0, 0, 0, 0, 1, -1};

void World::setBlock(int x, int y, int z, unsigned char block) {
	level->setBlock(x, y, z, block);
	for(int i = 0; i < 6; i++)
	{
		Chunk* c = getChunk((x+dx[i])/CHUNKSIZE, (y+dy[i])/CHUNKSIZE, (z+dz[i])/CHUNKSIZE);
		if(c != nullptr)
			c->markForRedraw();
	}
}
