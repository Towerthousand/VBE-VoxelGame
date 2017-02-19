#include "Biome.hpp"

const std::valarray<float> BIOME_PARAMS[NUM_BIOMES] = {
    //OCEAN
    {
        145.0f, // SIMPLEX_LOW
        155.0f, // SIMPLEX_HIGH
        4.0f,   // LOW_SURFACE_DEPTH
        1.0f,   // SURFACE_DEPTH
    },

    //PLAINS
    {
        160.0f, // SIMPLEX_LOW
        170.0f, // SIMPLEX_HIGH
        4.0f,   // LOW_SURFACE_DEPTH
        1.0f,   // SURFACE_DEPTH
    }
};

const std::valarray<int> BIOME_INT_PARAMS[NUM_BIOMES] = {
    //OCEAN
    {
        2,   // MAIN_TERRAIN
        8,   // LOW_SURFACE
        8,   // SURFACE
        2,   // TREE_MIN_GRID
        5,   // TREE_MAX_GRID
        4,   // TREE_CUTOFF
        100, // TREE_DROP_CHANCE
    },
    //PLAINS
    {
        2,   // MAIN_TERRAIN
        1,   // LOW_SURFACE
        3,   // SURFACE
        2,   // TREE_MIN_GRID
        5,   // TREE_MAX_GRID
        4,   // TREE_CUTOFF
        0,   // TREE_DROP_CHANCE
    }
};
