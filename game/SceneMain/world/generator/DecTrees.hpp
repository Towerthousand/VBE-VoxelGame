#ifndef DECTREES_HPP
#define DECTREES_HPP
#include "Dec.hpp"
#include "Noise2D.hpp"

class DecTrees final : public Dec {
    public:
        DecTrees(std::mt19937* rng) :
            gridNoise(rng, 0.0f, 1.0f, 60.0f),
            dispNoiseX(rng, -0.5f, 0.5f, 0.1f),
            dispNoiseY(rng, -0.5f, 0.5f, 0.1f),
            //dispNoiseX(rng, 0.0f, 0.0f, 0.1f),
            //dispNoiseY(rng, 0.0f, 0.0f, 0.1f),
            dropNoise(rng, 0.0f, 1.0f, 100.0f) {}
        virtual ~DecTrees() {}

        void decorate(ColumnGenerator::ColumnData* col) {
            std::vector<vec2i> trees;
            vec2i offset = col->pos*CHUNKSIZE;
            int minGridSize = 3;
            int maxGridSize = 5;
            int gridSizeCutoff = 5;
            float dropChance = 0.5f;
            int margin = (1 << maxGridSize);
            for(int i = -margin; i < CHUNKSIZE+margin; ++i)
                for(int j = -margin; j < CHUNKSIZE+margin; ++j) {
                    vec2i c = vec2i(i,j) + offset;
                    // minGridSize <= gridsize <= maxGridSize
                    int gridsize = 1 << int(floor(minGridSize+gridNoise.octavedGet(c.x, c.y, 4)*((maxGridSize+1)-minGridSize)));

                    // Gridsize too big
                    if(gridsize >= (1 << gridSizeCutoff))
                        continue;
                    // For this gridsize, this is not the adequate
                    // position to displace
                    if(c%gridsize != vec2i(0))
                        continue;
                    // Random drop to make it look more patched
                    if(dropNoise.octavedGet(c.x, c.y, 4) < dropChance)
                        continue;

                    // Random displacement
                    vec2f disp = vec2f(dispNoiseX.get(c.x,c.y)*gridsize-1, dispNoiseY.get(c.x,c.y)*gridsize-1);

                    // The final position of the tree
                    vec2i p = c - offset + vec2i(glm::round(disp));

                    // Only processed if it falls in this chunk
                    if(p.x >= 0 && p.x < CHUNKSIZE && p.y >= 0 && p.y < CHUNKSIZE)
                        trees.push_back(p);
                }
            for(const vec2i& t : trees) {
                int top = (*col->raw)[0].size();
                int size = 10;

                // Just draw a generic column of wood for the moment..
                for(int i = top-1; i >= 0; --i) {
                    if((*col->raw)[t.x][i][t.y] != 0) {
                        for(int s = i+1; s < top && s < i+1+size; ++s)
                            col->setDecorationRC(t.x, s, t.y, 1, 6);
                        break;
                    }
                }
            }
        }
    private:
        Noise2D gridNoise;
        Noise2D dispNoiseX;
        Noise2D dispNoiseY;
        Noise2D dropNoise;
};

#endif // DECTREES_HPP
