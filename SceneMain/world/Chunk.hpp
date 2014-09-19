#ifndef CHUNK_HPP
#define CHUNK_HPP
#include "commons.hpp"

class World;
class DeferredContainer;
class Chunk {
	public:
		enum VisibilityFlag {
			MINX_MAXX = 0x0001,
			MINX_MINY = 0x0002,
			MINX_MAXY = 0x0004,
			MINX_MINZ = 0x0008,
			MINX_MAXZ = 0x0010,
			MINY_MAXX = 0x0020,
			MINY_MAXY = 0x0040,
			MINY_MINZ = 0x0080,
			MINY_MAXZ = 0x0100,
			MINZ_MAXX = 0x0200,
			MINZ_MAXY = 0x0400,
			MINZ_MAXZ = 0x0800,
			MAXX_MAXY = 0x1000,
			MAXX_MAXZ = 0x2000,
			MAXY_MAXZ = 0x4000,
			ALL_FLAGS = 0x7FFF
		};

		enum Face {
			MINX = 0x01,
			MINY = 0x02,
			MINZ = 0x04,
			MAXX = 0x08,
			MAXY = 0x10,
			MAXZ = 0x20,
			ALL_FACES = 0x3F
		};

		Chunk(int x, unsigned int y, int z);
		~Chunk();

		static void initStructures();
		static unsigned short getVisibilityFlagsForFaces(unsigned char faces);
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
		unsigned short visibilityFlags;
		mat4f modelMatrix;
		AABB boundingBox;
		Model terrainModel;
		Model boundingBoxModel;
		World* world;
		DeferredContainer* renderer;

		static const int textureIndexes[9][6];
		static std::vector<vec3c> visibilityNodes;
		static bool visited[CHUNKSIZE][CHUNKSIZE][CHUNKSIZE];
		static vec3c d[6];

		friend class ColumnGenerator;
		friend class Column;
};

#endif // CHUNK_HPP
