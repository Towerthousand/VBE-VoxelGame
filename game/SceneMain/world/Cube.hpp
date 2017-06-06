#ifndef CUBE_HPP
#define CUBE_HPP
#include "commons.hpp"

class Chunk;

namespace Cube {

struct __attribute__((packed)) Vert {
    Vert(unsigned char vx = 0, unsigned char vy = 0, unsigned char vz = 0,
                unsigned char n = 0,
                unsigned short tx = 0, unsigned short ty = 0, unsigned char l = 0) :
        vx(vx), vy(vy), vz(vz),
        n(n),
        tx(tx), ty(ty),
        l(l) {}
    unsigned char vx,vy,vz,n;
    unsigned short tx,ty;
    unsigned char l;
};

enum Type {
    AIR = 0,
    DIRT = 1,
    STONE = 2,
    GRASS = 3,
    LIGHT = 4,
    COBBLE = 5,
    LOG = 6,
    PLANKS = 7,
    SAND = 8,
    LEAVES = 9,
    WATER = 10,
    NUM_TYPES,
};

enum PropertyFlag {
    TRANSPARENT = (1 << 0),
    SOLID       = (1 << 1),
};

constexpr unsigned int PROPERTIES[NUM_TYPES] = {
    TRANSPARENT, // 0  AIR
    SOLID,       // 1  DIRT
    SOLID,       // 2  STONE
    SOLID,       // 3  GRASS
    SOLID,       // 4  LIGHT
    SOLID,       // 5  COBBLE
    SOLID,       // 6  LOG
    SOLID,       // 7  PLANKS
    SOLID,       // 8  SAND
    SOLID,       // 9  LEAVES
    TRANSPARENT, // 10 WATER
};

constexpr unsigned char OPACITY[NUM_TYPES] = {
    0,   // 0  AIR
    0,   // 1  DIRT
    0,   // 2  STONE
    0,   // 3  GRASS
    0,   // 4  LIGHT
    0,   // 5  COBBLE
    0,   // 6  LOG
    0,   // 7  PLANKS
    0,   // 8  SAND
    0,   // 9  LEAVES
    128, // 10 WATER
};

constexpr unsigned int TEXTURE_INDEXES[NUM_TYPES][6] = { //order is front, back, left, right, bottom, top
    {0, 0, 0, 0, 0, 0},       //0  AIR (empty, will never be used)
    {2, 2, 2, 2, 2, 2},       //1  DIRT
    {3, 3, 3, 3, 3, 3},       //2  STONE
    {0, 0, 0, 0, 2, 1},       //3  GRASS
    {4, 4, 4, 4, 4, 4},       //4  LIGHT
    {5, 5, 5, 5, 5, 5},       //5  COBBLE
    {6, 6, 6, 6, 7, 7},       //6  LOG
    {8, 8, 8, 8, 8, 8},       //7  PLANKS
    {9, 9, 9, 9, 9, 9},       //8  SAND
    {10, 10, 10, 10, 10, 10}, //9  LEAVES
    {11, 11, 11, 11, 11, 11}, //10 WATER
};

constexpr int TEXSIZE = 8;
constexpr int ATLAS_SIZE = 512;

inline bool getFlag(Type t, PropertyFlag f) {
    return bool(PROPERTIES[t] & f);
};

void pushCubeToArray(short x, short y, short z, Cube::Type cubeID, const Chunk* c, std::vector<Vert>& renderData);

} // namespace Cube

#endif // CUBE_HPP
