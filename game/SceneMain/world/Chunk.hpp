#ifndef CHUNK_HPP
#define CHUNK_HPP
#include "commons.hpp"
#include "World.hpp"

#define AO_MAX_RAD 9

class DeferredContainer;
class Chunk {
	public:
		enum Face {
			MINX = 0,
			MAXX = 1,
			MINY = 2,
			MAXY = 3,
			MINZ = 4,
			MAXZ = 5,
			ALL_FACES = 6
		};

		Chunk(int x, unsigned int y, int z);
		~Chunk();

		static void initStructures();
		static Face getOppositeFace(Face f);

		void update(float deltaTime);
		void draw() const;
		void rebuildMesh();

		inline int getX() const { return XPOS; }
		inline unsigned int getY() const { return YPOS; }
		inline int getZ() const { return ZPOS; }
		inline vec3i getAbsolutePos() const { //in cubes
			return vec3i(XPOS*CHUNKSIZE, YPOS*CHUNKSIZE, ZPOS*CHUNKSIZE);
		}
		inline AABB getWorldSpaceBoundingBox() const {
			vec3f absPos = vec3f(getAbsolutePos());
			return AABB(boundingBox.getMin()+absPos, boundingBox.getMax()+absPos);
		}
		bool visibilityTest(Chunk::Face exit) const;
		inline bool isEmpty() const { return (boundingBox.getDimensions() == vec3f(0)); }
		bool isSurrounded() const;
		inline bool hasMesh() const { return hasVertices; }
		inline bool wasDrawedByPlayer() const { return drawedByPlayer; }

		std::bitset<6> facesVisited;

	private:
        void calcLightSum();
        int sumRect(int x1, int y1, int z1, int x2, int y2, int z2);
        float calcSubLight(int x, int y, int z, int dx, int dy, int dz, int d);
        unsigned char calcLight(int x, int y, int z, int dx, int dy, int dz);
        struct __attribute__((packed)) Vert {
				Vert(unsigned char vx = 0, unsigned char vy = 0, unsigned char vz = 0,
							unsigned char n = 0,
                            unsigned short tx = 0, unsigned short ty = 0, unsigned short l = 0) :
					vx(vx), vy(vy), vz(vz),
					n(n),
					tx(tx), ty(ty),
                    l(l) {}
				unsigned char vx,vy,vz,n;
				unsigned short tx,ty;
				unsigned char l;
		};

		static inline int getVisibilityIndex(int a, int b) {
			if(a >= b && a > 0) a--;
			return a+b*5;
		}

		void initMesh();
		void rebuildVisibilityGraph();
		inline unsigned int getCube(int x, int y, int z) const { //local coords, (0,0,0) is (XPOS*CS,YPOS*CS,ZPOS*CS) in absolute
			if(x >= 0 && x < CHUNKSIZE && y >= 0 && y < CHUNKSIZE && z >= 0 && z < CHUNKSIZE)
				return cubes[x][y][z];
			return world->getCube(x+(XPOS*CHUNKSIZE), y+(YPOS*CHUNKSIZE), z+(ZPOS*CHUNKSIZE)); //in another chunk
		}
		void pushCubeToArray(short x, short y, short z, std::vector<Vert>& renderData);

		unsigned int cubes[CHUNKSIZE][CHUNKSIZE][CHUNKSIZE];
		const int XPOS; //in chunks
		const unsigned int YPOS; //in chunks
		const int ZPOS; //in chunks
		mutable bool drawedByPlayer; //has been seen by a player this frame?
		bool needsMeshRebuild; //does it need rebuilding?
		bool hasVertices; //is there any face touching air?
		std::bitset<30> visibilityGraph;
		AABB boundingBox;
		MeshBatched* terrainModel;
		MeshIndexed* boundingBoxModel;
		World* world;
		DeferredContainer* renderer;

		static const int textureIndexes[9][6];
		static std::vector<vec3c> visibilityNodes;
		static vec3c d[6];

		friend class ColumnGenerator;
		friend class Column;
};

#endif // CHUNK_HPP
