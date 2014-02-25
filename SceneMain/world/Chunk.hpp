#ifndef CHUNK_HPP
#define CHUNK_HPP
#include "commons.hpp"

class World;
class Chunk {
	public:
		Chunk(int x, unsigned int y, int z);
		~Chunk();

		void update(float deltaTime);
		void draw() const;
		void drawBoundingBox() const;

		int getX() const { return XPOS; }
		unsigned int getY() const { return YPOS; }
		int getZ() const { return ZPOS; }
		vec3i getAbsolutePos() const; //in cubes
		AABB getWorldSpaceBoundingBox();
		unsigned int isHidden() const { return (boundingBox.getDimensions() == vec3f(0)); }

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

		unsigned int getCube(int x, int y, int z) const; //local coords, (0,0,0) is (XPOS*CS,YPOS*CS,ZPOS*CS) in absolute
		void pushCubeToArray(short x, short y, short z, std::vector<Vert>& renderData);
		void initMesh();

		unsigned int cubes[CHUNKSIZE][CHUNKSIZE][CHUNKSIZE];
		const int XPOS; //in chunks
		const unsigned int YPOS; //in chunks
		const int ZPOS; //in chunks
		bool markedForRedraw;

		mat4f modelMatrix;
		AABB boundingBox;
		Model terrainModel;
		Model boundingBoxModel;

		World* world;

		friend class ColumnGenerator;
		friend class Column;
};

#endif // CHUNK_HPP
