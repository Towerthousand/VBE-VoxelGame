#include "World.hpp"
#include "Column.hpp"
#include "Chunk.hpp"
#include "../DeferredContainer.hpp"
#include "Sun.hpp"
#include "SceneMain/debug/Profiler.hpp"
#include "../Manager.hpp"
#include <cstring>

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
	float worldUpdateTime = Clock::getSeconds();
	generator.discardTasks();
	Column* newCol = nullptr;
	while((newCol = generator.pullDone()) != nullptr) {
		if(getColumnCC(newCol->getX(), 0, newCol->getZ()) != nullptr) delete newCol;
		else  {
			Profiler::intVars[Profiler::ColumnsAdded]++;
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
	Profiler::timeVars[Profiler::WorldUpdateTime] = Clock::getSeconds() - worldUpdateTime;
}

void World::draw() const {
	switch(renderer->getMode()) {
		case DeferredContainer::Deferred:
			draw((Camera*)getGame()->getObjectByName("playerCam"));
			break;
		case DeferredContainer::ShadowMap: {
			Sun* sun = (Sun*)getGame()->getObjectByName("sun");
			sun->updateCameras();
			draw(sun->getGlobalCam());
			break;
		}
		default: break;
	}
}

void World::draw(const Camera* cam) const{
	struct Job {
			vec3i pos;
			int distance;
	};
	static Chunk::Face faces[6] = {
		Chunk::MINX, Chunk::MAXX,
		Chunk::MINY, Chunk::MAXY,
		Chunk::MINZ, Chunk::MAXZ
	};
	static vec3i offsets[6] = {
		vec3i(-1,0,0), vec3i(1,0,0),
		vec3i(0,-1,0), vec3i(0,1,0),
		vec3i(0,0,-1), vec3i(0,0,1)
	};
	float chunkRebuildTime = 0.0f;
	float chunkBFSTime = Clock::getSeconds();
	std::list<vec3i> chunksToDraw; //push all the chunks that must be drawn here
	vec3i initialChunkPos(glm::floor(cam->getWorldPos())/float(CHUNKSIZE));
	std::queue<Job> q; //bfs queue, each node is (entry face, chunkPos, distance to source), in chunk coords
	for(int i = 0; i < 6; ++i) {
		Chunk* aux;
		//push chunk with current face
		q.push({initialChunkPos, 0});
		aux = getChunkCC(initialChunkPos); //used later inside neighbor loop
		if(aux != nullptr) aux->facesVisited.set(i);
	}
	Sphere colliderSphere(vec3f(0.0f), (CHUNKSIZE>>1)*1.74f);
	vec3i colliderOffset = vec3i(CHUNKSIZE >> 1);
	std::bitset<6> ommitedPlanes;
	if(renderer->getMode() == DeferredContainer::ShadowMap) ommitedPlanes.set(Frustum::NEAR);
	static bool visited[WORLDSIZE][WORLDSIZE][WORLDSIZE];
	memset(visited, 0, sizeof(visited));
	//bfs
	while(!q.empty()) {
		Job currentJob = q.front();
		q.pop();
		//Process current chunk
		if(visited[currentJob.pos.x & WORLDSIZE_MASK][currentJob.pos.y & WORLDSIZE_MASK][currentJob.pos.z & WORLDSIZE_MASK]) continue;
		visited[currentJob.pos.x & WORLDSIZE_MASK][currentJob.pos.y & WORLDSIZE_MASK][currentJob.pos.z & WORLDSIZE_MASK] = true;
		Chunk* currentChunk = getChunkCC(currentJob.pos); //used later inside neighbor loop
		if(currentChunk != nullptr) {
			float chunkRebuildTime0 = Clock::getSeconds();
			currentChunk->rebuildMesh();
			chunkRebuildTime += Clock::getSeconds() - chunkRebuildTime0;
			chunksToDraw.push_back(currentJob.pos);
		}
		int distance = currentJob.distance+1; //neighbor chunks's bfs distance
		//foreach face
		for(int i = 0; i < 6; ++i) {
			Job neighborJob = {currentJob.pos + offsets[i], distance};
			//manhattan culling
			if(distance > Utils::manhattanDistance(initialChunkPos, neighborJob.pos)) continue;
			//out-of-bounds culling (null column, we do explore null chunks since they may be anywhere)
			if((neighborJob.pos.y >= (int)highestChunkY && faces[i] == Chunk::MAXY) || (getColumnCC(neighborJob.pos) == nullptr && distance > WORLDSIZE/2)) continue;
			//visibility culling
			if(currentChunk != nullptr && !currentChunk->visibilityTest(faces[i])) continue;
			//fustrum culling
			colliderSphere.center = neighborJob.pos*CHUNKSIZE+colliderOffset;
			if(!Collision::intersects(cam->getFrustum(), colliderSphere, ommitedPlanes)) continue;
			//push it
			Chunk* neighborChunk = getChunkCC(neighborJob.pos);
			if(neighborChunk != nullptr) neighborChunk->facesVisited.set(Chunk::getOppositeFace(faces[i]));
			q.push(neighborJob);
		}
	}
	float chunkDrawTime = Clock::getSeconds();
	chunkBFSTime = chunkDrawTime-chunkBFSTime-chunkRebuildTime;
	Sun* sun = ((Sun*)getGame()->getObjectByName("sun"));
	if(renderer->getMode() == DeferredContainer::Deferred)  {
		const Camera* cam2 = cam;
		if(Keyboard::pressed(Keyboard::Q)) cam2 = sun->getGlobalCam();
		Programs.get("deferredChunk").uniform("V")->set(cam2->getView());
		Programs.get("deferredChunk").uniform("VP")->set(cam2->projection*cam2->getView());
		Programs.get("deferredChunk").uniform("diffuseTex")->set(Textures2D.get("blocks"));
	}
	else {
		Sun* sun = (Sun*)getGame()->getObjectByName("sun");
		Programs.get("depthShader").uniform("VP")->set(sun->getVPMatrices());
	}
	static std::vector<vec3i> transforms(400);
	int chunkCount = 0;
	int batchCount = 0;
	MeshBatched::startBatch();
	for(const vec3i& pos : chunksToDraw) {
		Chunk* c = getChunkCC(pos);
		if(c != nullptr) {
			c->facesVisited.reset();
			if(renderer->getMode() == DeferredContainer::ShadowMap) {
				const Frustum& f = ((Camera*)getGame()->getObjectByName("playerCam"))->getFrustum();
				colliderSphere.center = c->getAbsolutePos()+colliderOffset;
				bool pass = false;
				for(int i = 0; i < 6; ++i) {
					Plane p = f.getPlane((Frustum::PlaneID)i);
					if(!p.inside(colliderSphere) && glm::dot(p.n, sun->getDirection()) > 0)
						pass = true;
				}
				if(pass) continue;
			}
			c->draw();
			transforms[batchCount] = c->getAbsolutePos();
			++chunkCount;
			++batchCount;
		}
		if(batchCount == 400) {
			batchCount -= 400;
			if(renderer->getMode() == DeferredContainer::Deferred)
				Programs.get("deferredChunk").uniform("transforms")->set(transforms);
			else
				Programs.get("depthShader").uniform("transforms")->set(transforms);
			MeshBatched::endBatch();
			MeshBatched::startBatch();
		}
	}
	if(batchCount > 0) {
		if(renderer->getMode() == DeferredContainer::Deferred)
			Programs.get("deferredChunk").uniform("transforms")->set(transforms);
		else
			Programs.get("depthShader").uniform("transforms")->set(transforms);
	}
	MeshBatched::endBatch();

	switch(renderer->getMode()) {
		case DeferredContainer::Deferred:
			Profiler::timeVars[Profiler::PlayerChunkDrawTime] = Clock::getSeconds() - chunkDrawTime;
			Profiler::timeVars[Profiler::PlayerChunkRebuildTime] = chunkRebuildTime;
			Profiler::timeVars[Profiler::PlayerChunkBFSTime] = chunkBFSTime;
			Profiler::intVars[Profiler::PlayerChunksDrawn] = chunkCount; break;
		case DeferredContainer::ShadowMap:
			Profiler::timeVars[Profiler::ShadowChunkDrawTime] = Clock::getSeconds() - chunkDrawTime;
			Profiler::timeVars[Profiler::ShadowChunkRebuildTime] = chunkRebuildTime;
			Profiler::timeVars[Profiler::ShadowChunkBFSTime] = chunkBFSTime;
			Profiler::intVars[Profiler::SunChunksDrawn] = chunkCount;
			break;
		default: break;
	}
}
