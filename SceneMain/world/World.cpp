#include "World.hpp"
#include "Column.hpp"
#include "Chunk.hpp"
#include "../DeferredContainer.hpp"
#include "Sun.hpp"
#include "SceneMain/debug/Profiler.hpp"

struct FunctorComparevec3iFace{
		bool operator()(const std::pair<Chunk::Face, vec3i>& a, const std::pair<Chunk::Face, vec3i>& b) {
			if(a.second.x != b.second.x) return a.second.x < b.second.x;
			if(a.second.y != b.second.y) return a.second.y < b.second.y;
			if(a.second.z != b.second.z) return a.second.z < b.second.z;
			if(a.first != b.first) return (int)a.first < (int)b.first;
			return false;
		}
};

World::World() : generator(rand()), renderer(nullptr) {
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
	generator.discardTasks();
	Column* newCol = nullptr;
	while((newCol = generator.pullDone()) != nullptr) {
		if(columns[newCol->getX()&WORLDSIZE_MASK][newCol->getZ()&WORLDSIZE_MASK] != nullptr) delete newCol;
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
	std::set<Chunk*> chunksToDraw; //push all the chunks that must be drawn here
	vec3i initialChunk(glm::floor(cam->getWorldPos())/float(CHUNKSIZE));
	std::queue<std::pair<Chunk::Face, vec3i>>q; //bfs queue, each node is (entry face, chunkPos), in chunk coords
	std::map<std::pair<Chunk::Face, vec3i>,int, FunctorComparevec3iFace> visited; //visited nodes.
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
	for(int i = 0; i < 6; ++i) {
		//push chunk with current face
		q.push(std::pair<Chunk::Face, vec3i>(faces[i], initialChunk));
		visited.insert(std::pair<std::pair<Chunk::Face, vec3i>, int>(std::pair<Chunk::Face, vec3i>(faces[i], initialChunk), 0));
		//initial neighbor for this face
		q.push(std::pair<Chunk::Face, vec3i>(Chunk::getOppositeFace(faces[i]), initialChunk+offsets[i]));
		visited.insert(std::pair<std::pair<Chunk::Face, vec3i>, int>(std::pair<Chunk::Face, vec3i>(Chunk::getOppositeFace(faces[i]), initialChunk+offsets[i]), 1));
	}
	//bfs
	while(!q.empty()) {
		std::pair<Chunk::Face, vec3i> currentChunkPos = q.front();
		q.pop();

		//Process current chunk: if it exists, rebuild mesh+visibility graph and queue for drawing
		Chunk* currentChunk = getChunk(currentChunkPos.second*CHUNKSIZE); //used later inside neighbor loop
		if(currentChunk != nullptr) {
			currentChunk->rebuildMesh();
			chunksToDraw.insert(currentChunk);
		}
		int distance = visited.at(currentChunkPos)+1; //neighbor chunks's bfs distance
		//foreach face
		for(int i = 0; i < 6; ++i) {
			std::pair<Chunk::Face,vec3i> neighborChunkPos(Chunk::getOppositeFace(faces[i]), currentChunkPos.second + offsets[i]);

			//visited culling
			if(visited.find(neighborChunkPos) != visited.end()) continue;
			//manhattan culling
			if(distance > Utils::manhattanDistance(initialChunk, neighborChunkPos.second)) continue;
			//out-of-bounds culling (null column, we do explore null chunks since they may be anywhere)
			if(getColumn(neighborChunkPos.second*CHUNKSIZE) == nullptr) continue;
			//visibility culling
			if(currentChunk != nullptr && !currentChunk->visibilityTest(currentChunkPos.first , faces[i])) continue;
			//fustrum culling
			if(!Collision::intersects(cam->getFrustum(), Sphere(vec3f(neighborChunkPos.second*CHUNKSIZE+vec3i(CHUNKSIZE >> 1)), (CHUNKSIZE>>1)*1.74f))) continue;

			visited.insert(std::pair<std::pair<Chunk::Face, vec3i>, int>(neighborChunkPos, distance));
			q.push(neighborChunkPos);
		}
	}
	for(auto it = chunksToDraw.begin(); it != chunksToDraw.end(); ++it)
		(*it)->draw();

	switch(renderer->getMode()) {
		case DeferredContainer::Deferred:
			Profiler::cameraChunksDrawn = chunksToDraw.size(); break;
		case DeferredContainer::ShadowMap:
			Profiler::sunChunksDrawn = chunksToDraw.size(); break;
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

unsigned int World::getCube(int x, int y, int z) const {
	Column* c = getColumn(x,y,z);
	return (c == nullptr)? 0 : c->getCube(x & CHUNKSIZE_MASK,y,z & CHUNKSIZE_MASK);
}

void World::setCube(int x, int y, int z, unsigned int cube) {
	Column* c = getColumn(x,y,z);
	if(c == nullptr) return;
	c->setCube(x & CHUNKSIZE_MASK, y, z & CHUNKSIZE_MASK, cube);
}
