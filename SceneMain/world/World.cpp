#include "World.hpp"
#include "Column.hpp"
#include "Chunk.hpp"
#include "../DeferredContainer.hpp"
#include "Sun.hpp"
#include "SceneMain/debug/Profiler.hpp"

World::World() : highestChunkY(0), generator(rand()), renderer(nullptr) {
	renderer = (DeferredContainer*)getGame()->getObjectByName("deferred");
	setName("world");
	for(int x = 0; x < WORLDSIZE; ++x)
		for(int z = 0; z < WORLDSIZE; ++z)
			columns[x][z] = nullptr;
	Sun* sun = new Sun();
	sun->addTo(this);
	Chunk::initStructures();
}

World::~World() {
}

void World::update(float deltaTime) {
	float worldUpdateTime = Environment::getClock();
	generator.discardTasks();
	Column* newCol = nullptr;
	while((newCol = generator.pullDone()) != nullptr) {
		if(getColumnCC(newCol->getX(), 0, newCol->getZ()) != nullptr) delete newCol;
		else  {
			columns[newCol->getX()&WORLDSIZE_MASK][newCol->getZ()&WORLDSIZE_MASK] = newCol;
			if(getColumn(newCol->getAbolutePos()+vec3i(CHUNKSIZE,0,0)) != nullptr) getColumn(newCol->getAbolutePos()+vec3i(CHUNKSIZE,0,0))->rebuildMeshes();
			if(getColumn(newCol->getAbolutePos()-vec3i(CHUNKSIZE,0,0)) != nullptr) getColumn(newCol->getAbolutePos()-vec3i(CHUNKSIZE,0,0))->rebuildMeshes();
			if(getColumn(newCol->getAbolutePos()+vec3i(0,0,CHUNKSIZE)) != nullptr) getColumn(newCol->getAbolutePos()+vec3i(0,0,CHUNKSIZE))->rebuildMeshes();
			if(getColumn(newCol->getAbolutePos()-vec3i(0,0,CHUNKSIZE)) != nullptr) getColumn(newCol->getAbolutePos()-vec3i(0,0,CHUNKSIZE))->rebuildMeshes();
		}
	}
	Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");
	vec2f playerChunkPos = vec2f(vec2i(cam->getWorldPos().x,cam->getWorldPos().z)/CHUNKSIZE);
	std::vector<std::pair<float,std::pair<int,int> > > tasks;
	highestChunkY = 0;
	for(int x = -WORLDSIZE/2; x < WORLDSIZE/2; ++x)
		for(int z = -WORLDSIZE/2; z < WORLDSIZE/2; ++z) {
			vec2f colPos = playerChunkPos + vec2f(x,z);
			Column* actual = getColumnCC(colPos.x,0,colPos.y);
			if(actual == nullptr && !generator.currentlyWorking(vec2i(colPos))) {
				Column*& realpos = columns[int(colPos.x)&WORLDSIZE_MASK][int(colPos.y)&WORLDSIZE_MASK];
				if(realpos != nullptr) delete realpos;
				tasks.push_back(std::pair<float,std::pair<int,int> >(glm::length(playerChunkPos-colPos),std::pair<int,int>(colPos.x,colPos.y)));
				realpos = nullptr;
				continue;
			}
			if(actual == nullptr) continue;
			highestChunkY = std::max(actual->getChunkCount(), highestChunkY);
			for(unsigned int y = 0; y < actual->getChunkCount(); ++y){
				Chunk* c = actual->getChunkCC(y);
				if(c == nullptr) continue;
				c->update(deltaTime);
			}
		}
	std::sort(tasks.begin(),tasks.end());
	for(unsigned int i = 0; i < tasks.size(); ++i) generator.enqueueTask(vec2i(tasks[i].second.first,tasks[i].second.second));
	Profiler::timeVars[Profiler::WorldUpdateTime] = Environment::getClock() - worldUpdateTime;
}

void World::draw() const {
	switch(renderer->getMode()) {
		case DeferredContainer::Deferred:
			draw((Camera*)getGame()->getObjectByName("playerCam"));
			break;
		case DeferredContainer::ShadowMap:
			((Sun*)getGame()->getObjectByName("sun"))->updateCamera();
			draw((Camera*)getGame()->getObjectByName("sunCamera"));
			break;
		default: break;
	}
}

void World::draw(Camera* cam) const{
	struct Hasher {
			std::size_t operator()(const vec3i& a) const {
				return 961*a.y + 31*a.z + a.x;
			}
	};
	struct Job {
			vec3i pos;
			int distance;
	};
	Chunk::Face faces[6] = {
		Chunk::MINX, Chunk::MAXX,
		Chunk::MINY, Chunk::MAXY,
		Chunk::MINZ, Chunk::MAXZ
	};
	vec3i offsets[6] = {
		vec3i(-1,0,0), vec3i(1,0,0),
		vec3i(0,-1,0), vec3i(0,1,0),
		vec3i(0,0,-1), vec3i(0,0,1)
	};
	float chunkRebuildTime = 0.0f;
	float chunkBFSTime = Environment::getClock();
	std::unordered_set<vec3i, Hasher> chunksToDraw; //push all the chunks that must be drawn here
	vec3i initialChunkPos(glm::floor(cam->getWorldPos())/float(CHUNKSIZE));
	std::queue<Job> q; //bfs queue, each node is (entry face, chunkPos, distance to source), in chunk coords
	for(int i = 0; i < 6; ++i) {
		Chunk* aux;
		//push chunk with current face
		q.push({initialChunkPos, 0});
		aux = getChunkCC(initialChunkPos); //used later inside neighbor loop
		if(aux != nullptr) aux->facesVisited.set(i);
		//initial neighbor for this face
		q.push({initialChunkPos+offsets[i], 1});
		aux = getChunkCC(initialChunkPos+offsets[i]); //used later inside neighbor loop
		if(aux != nullptr) aux->facesVisited.set(Chunk::getOppositeFace(faces[i]));
	}
	Sphere colliderSphere(vec3f(0.0f), (CHUNKSIZE>>1)*1.74f);
	vec3i colliderOffset = vec3i(CHUNKSIZE >> 1);
	//bfs
	while(!q.empty()) {
		Job currentJob = q.front();
		q.pop();
		//Process current chunk
		if(!chunksToDraw.insert(currentJob.pos).second) continue;
		Chunk* currentChunk = getChunkCC(currentJob.pos); //used later inside neighbor loop
		if(currentChunk != nullptr) {
			float chunkRebuildTime0 = Environment::getClock();
			currentChunk->rebuildMesh();
			chunkRebuildTime += Environment::getClock() - chunkRebuildTime0;
		}
		int distance = currentJob.distance+1; //neighbor chunks's bfs distance
		//foreach face
		for(int i = 0; i < 6; ++i) {
			Job neighborJob = {currentJob.pos + offsets[i], distance};
			//manhattan culling
			if(distance > Utils::manhattanDistance(initialChunkPos, neighborJob.pos)) continue;
			//out-of-bounds culling (null column, we do explore null chunks since they may be anywhere)
			if((neighborJob.pos.y >= (int)highestChunkY && i == 3) || getColumnCC(neighborJob.pos) == nullptr) continue;
			//visibility culling
			if(currentChunk != nullptr && !currentChunk->visibilityTest(faces[i])) continue;
			//fustrum culling
			colliderSphere.center = neighborJob.pos*CHUNKSIZE+colliderOffset;
			if(!Collision::intersects(cam->getFrustum(), colliderSphere)) continue;
			//push it
			Chunk* neighborChunk = getChunkCC(neighborJob.pos);
			if(neighborChunk != nullptr) neighborChunk->facesVisited.set(Chunk::getOppositeFace(faces[i]));
			q.push(neighborJob);
		}
	}
	float chunkDrawTime = Environment::getClock();
	chunkBFSTime = chunkDrawTime-chunkBFSTime-chunkRebuildTime;
	int chunkCount = 0;
	for(auto it = chunksToDraw.begin(); it != chunksToDraw.end(); ++it) {
		Chunk* c = getChunkCC(*it);
		if(c != nullptr) {
			c->facesVisited.reset();
			c->draw();
			chunkCount++;
		}
	}

	switch(renderer->getMode()) {
		case DeferredContainer::Deferred:
			Profiler::timeVars[Profiler::PlayerChunkDrawTime] = Environment::getClock() - chunkDrawTime;
			Profiler::timeVars[Profiler::PlayerChunkRebuildTime] = chunkRebuildTime;
			Profiler::timeVars[Profiler::PlayerChunkBFSTime] = chunkBFSTime;
			Profiler::intVars[Profiler::PlayerChunksDrawn] = chunkCount; break;
		case DeferredContainer::ShadowMap:
			Profiler::timeVars[Profiler::SunChunkDrawTime] = Environment::getClock() - chunkDrawTime;
			Profiler::timeVars[Profiler::SunChunkRebuildTime] = chunkRebuildTime;
			Profiler::timeVars[Profiler::SunChunkBFSTime] = chunkBFSTime;
			Profiler::intVars[Profiler::SunChunksDrawn] = chunkCount;
			break;
		default: break;
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

Chunk* World::getChunk(int x, int y, int z) const {
	Column* c = getColumn(x,y,z);
	return c == nullptr ? nullptr : c->getChunk(y);
}

Column* World::getColumnCC(int x, int y, int z) const {
	Column* c = columns[x & WORLDSIZE_MASK][z & WORLDSIZE_MASK];
	return (c == nullptr || c->getX() != x || c->getZ() != z || y < 0)? nullptr:c;
}

Chunk* World::getChunkCC(int x, int y, int z) const {
	Column* c = getColumnCC(x,y,z);
	return c == nullptr ? nullptr : c->getChunkCC(y);
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
