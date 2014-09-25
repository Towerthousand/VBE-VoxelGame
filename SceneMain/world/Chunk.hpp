#ifndef CHUNK_HPP
#define CHUNK_HPP
#include "commons.hpp"

class World;
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
		void drawBoundingBox() const;
		void rebuildMesh();

		int getX() const { return XPOS; }
		unsigned int getY() const { return YPOS; }
		int getZ() const { return ZPOS; }
		vec3i getAbsolutePos() const; //in cubes
		AABB getWorldSpaceBoundingBox() const;
		bool visibilityTest(Chunk::Face enter, Chunk::Face exit) const;
		bool isEmpty() const { return (boundingBox.getDimensions() == vec3f(0)); }
		bool hasMesh() const { return hasVertices; }

	private:
		struct Vert {
				Vert(unsigned char vx = 0, unsigned char vy = 0, unsigned char vz = 0,
							unsigned char n = 0,
							unsigned short tx = 0, unsigned short ty = 0) :
					vx(vx), vy(vy), vz(vz),
					n(n),
					tx(tx), ty(ty) {}
				unsigned char vx,vy,vz,n;
				unsigned short tx,ty;
		};

		static int getVisibilityIndex(int a, int b);

		void initMesh();
		void rebuildVisibilityGraph();
		unsigned int getCube(int x, int y, int z) const; //local coords, (0,0,0) is (XPOS*CS,YPOS*CS,ZPOS*CS) in absolute
		void pushCubeToArray(short x, short y, short z, std::vector<Vert>& renderData);

		unsigned int cubes[CHUNKSIZE][CHUNKSIZE][CHUNKSIZE];
		const int XPOS; //in chunks
		const unsigned int YPOS; //in chunks
		const int ZPOS; //in chunks
		bool needsMeshRebuild; //does it need rebuilding?
		bool hasVertices; //is there any face touching air?
		std::bitset<30> visibilityGraph;
		mat4f modelMatrix;
		AABB boundingBox;
		Model terrainModel;
		Model boundingBoxModel;
		World* world;
		DeferredContainer* renderer;

		static const int textureIndexes[9][6];
		static std::vector<vec3c> visibilityNodes;
		static vec3c d[6];

		friend class ColumnGenerator;
		friend class Column;
};

#endif // CHUNK_HPP
