#ifndef CHUNK_HPP
#define CHUNK_HPP
#include "commons.hpp"
#include "Cube.hpp"

struct ChunkVertex {
		ChunkVertex(unsigned char vx = 0, unsigned char vy = 0, unsigned char vz = 0,
					unsigned char n = 0,
					unsigned short tx = 0, unsigned short ty = 0,
					unsigned char cr = 255, unsigned char cg = 255, unsigned char cb = 255, unsigned char ca = 255) :
			vx(vx), vy(vy), vz(vz),
			n(n),
			tx(tx), ty(ty),
			cr(cr), cg(cg), cb(cb), ca(ca) {}
		unsigned char vx,vy,vz,n;
		unsigned short tx,ty;
		unsigned char cr,cg,cb,ca;
};

class World;
class Chunk {
	public:
		Chunk(int x, unsigned int y, int z);
		~Chunk();

		void update(float deltaTime);
		void draw() const;
		void drawBoundingBox() const;

		int getX() { return XPOS; }
		unsigned int getY() { return YPOS; }
		int getZ() { return ZPOS; }
		vec3i getAbsolutePos(); //in cubes

	private:
		Cube getCube(int x, int y, int z) const; //local coords, (0,0,0) is (XPOS*CS,YPOS*CS,ZPOS*CS) in absolute
		void pushCubeToArray(short x, short y, short z, std::vector<ChunkVertex>& renderData);

		Cube cubes[CHUNKSIZE][CHUNKSIZE][CHUNKSIZE];
		const int XPOS; //in chunks
		const unsigned int YPOS; //in chunks
		const int ZPOS; //in chunks
		mat4f modelMatrix;
		Model model;
		bool markedForRedraw;
		World* world;

		friend class ColumnGenerator;
		friend class Column;
};

#endif // CHUNK_HPP
