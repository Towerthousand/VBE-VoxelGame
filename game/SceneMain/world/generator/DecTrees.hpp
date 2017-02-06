#ifndef DECTREES_HPP
#define DECTREES_HPP
#include "Dec.hpp"
#include "terrainFunctions/Noise2D.hpp"

#define DECTREES_MIN_GRID_SIZE 2
#define DECTREES_MAX_GRID_SIZE 4
#define DECTREES_GRID_SIZE_CUTOFF 5
#define DECTREES_DROP_CHANCE 0.5f
#define DECTREES_MIN_POWER -0.5f
#define DECTREES_MAX_POWER 0.5f

class DecTrees final : public Dec {
    public:
        DecTrees(std::mt19937* rng) :
            gridNoise(rng, 0.0f, 1.0f, 60.0f),
            dispNoiseX(rng, DECTREES_MIN_POWER, DECTREES_MAX_POWER, 0.1f),
            dispNoiseY(rng, DECTREES_MIN_POWER, DECTREES_MAX_POWER, 0.1f),
            dropNoise(rng, 0.0f, 1.0f, 100.0f),
            genNoise(rng, 0.0f, 1.0f, 1.0f) {}
        virtual ~DecTrees() {}

        void decorate(ColumnGenerator::ColumnData* col) {
            std::vector<vec2i> treePositions = getTreePositions(col);
            for(const vec2i& t : treePositions)
                genTree(col, t);
        }

        void genTree(ColumnGenerator::ColumnData* col, const vec2i& pos) {
            // Create a generator for this specific world pos
            vec2i wPos = col->pos*CHUNKSIZE + pos;
            unsigned int seed = std::numeric_limits<unsigned int>::max()*genNoise.get(wPos.x, wPos.y);
            std::mt19937 gen(seed);

            int top = (*col->raw)[0].size();

            // Tree height
            int tHeight = 8+gen()%8;

            // Find Base
            unsigned int base = 0;
            for(int i = top-1; i >= 0; --i)
                if((*col->raw)[pos.x][i][pos.y] != 0) {
                    base = i;
                    break;
                }

            struct Branch {
                unsigned int height, length, dir;
            };

            // directions a branch for a given dir [n, s, e, w]
            // can branch in
            static const vec3i bDirs[4][9] = {
                {
                    { 0, 0, 1},{ 1, 0, 1},{-1, 0, 1},
                    { 0, 1, 1},{ 1, 1, 1},{-1, 1, 1},
                    { 0,-1, 1},{ 1,-1, 1},{-1,-1, 1},
                },
                {
                    { 0, 0,-1},{ 1, 0,-1},{-1, 0,-1},
                    { 0, 1,-1},{ 1, 1,-1},{-1, 1,-1},
                    { 0,-1,-1},{ 1,-1,-1},{-1,-1,-1},
                },
                {
                    { 1, 0, 0},{ 1, 0, 1},{ 1, 0,-1},
                    { 1, 1, 0},{ 1, 1, 1},{ 1, 1,-1},
                    { 1,-1, 0},{ 1,-1, 1},{ 1,-1,-1},
                },
                {
                    {-1, 0, 0},{-1, 0, 1},{-1, 0,-1},
                    {-1, 1, 0},{-1, 1, 1},{-1, 1,-1},
                    {-1,-1, 0},{-1,-1, 1},{-1,-1,-1},
                }
            };

            std::vector<Branch> branches(2+gen()%5);
            std::vector<vec3i> leafCubes;

            for(Branch& b : branches) {
                b.height = tHeight-4+gen()%5;
                b.length = 1+gen()%5;
                b.dir = gen()%4;
            }

            // Draw trunk
            for(int i = 0; i < tHeight; ++i) {
                vec3i c = {pos.x, base+i+1, pos.y};
                col->setDecorationRC(c, 1, 6);
                if(tHeight-i < 5)
                    leafCubes.push_back(c);
            }

            // Draw branches
            for(const Branch& b : branches) {
                vec3i p = {pos.x, base+b.height, pos.y};
                for(unsigned int i = 1; i <= b.length; ++i) {
                    p += bDirs[b.dir][gen()%9];
                    col->setDecorationRC(p, 1, 6);
                    leafCubes.push_back(p);
                }
            }

            // Draw leaves
            for(const vec3i& c: leafCubes) {
                col->setDecorationRC(c+vec3i( 0, 0,  1), 1, 9);
                col->setDecorationRC(c+vec3i( 0, 0, -1), 1, 9);
                col->setDecorationRC(c+vec3i( 1, 0,  0), 1, 9);
                col->setDecorationRC(c+vec3i(-1, 0,  0), 1, 9);
                col->setDecorationRC(c+vec3i( 0, -1, 0), 1, 9);
                col->setDecorationRC(c+vec3i( 0, 1,  0), 1, 9);
            }
            for(const vec3i& c: leafCubes) {
                for(int dx = -1; dx <= 1; ++dx)
                    for(int dy = -1; dy <= 1; ++dy)
                        for(int dz = -1; dz <= 1; ++dz) {
                            // Randomly drop some of the outer leaves
                            // to look more detailed
                            int manhattanDist = abs(dx)+abs(dy)+abs(dz);
                            if((gen()%3 == 0 && manhattanDist > 1))
                                continue;
                            col->setDecorationRC(c+vec3i(dx, dy, dz), 1, 9);
                        }
            }
        }

        std::vector<vec2i> getTreePositions(ColumnGenerator::ColumnData* col) {
            std::vector<vec2i> trees;
            vec2i offset = col->pos*CHUNKSIZE;
            int margin = (1 << DECTREES_MAX_GRID_SIZE);
            for(int i = -margin; i < CHUNKSIZE+margin; ++i)
                for(int j = -margin; j < CHUNKSIZE+margin; ++j) {
                    vec2i c = vec2i(i,j) + offset;
                    // DECTREES_MIN_GRID_SIZE <= gridsize <= DECTREES_MAX_GRID_SIZE
                    int gridsize = 1 << int(floor(DECTREES_MIN_GRID_SIZE+gridNoise.octavedGet(c.x, c.y, 4)*((DECTREES_MAX_GRID_SIZE+1)-DECTREES_MIN_GRID_SIZE)));

                    // Gridsize too big
                    if(gridsize > (1 << DECTREES_GRID_SIZE_CUTOFF))
                        continue;
                    // For this gridsize, this is not the adequate
                    // position to displace
                    if(c%gridsize != vec2i(0))
                        continue;
                    // Random drop to make it look more patched
                    if(dropNoise.octavedGet(c.x, c.y, 4) < DECTREES_DROP_CHANCE)
                        continue;

                    // Random displacement
                    vec2f disp = vec2f(dispNoiseX.get(c.x,c.y)*gridsize-1, dispNoiseY.get(c.x,c.y)*gridsize-1);

                    // The final position of the tree
                    vec2i p = c - offset + vec2i(glm::round(disp));

                    // Only processed if it falls in this chunk
                    if(p.x >= 0 && p.x < CHUNKSIZE && p.y >= 0 && p.y < CHUNKSIZE)
                        trees.push_back(p);
                }
            return trees;
        }
    private:
        Noise2D gridNoise;
        Noise2D dispNoiseX;
        Noise2D dispNoiseY;
        Noise2D dropNoise;
        Noise2D genNoise;
};

#endif // DECTREES_HPP
