#include "Biome.hpp"

const std::valarray<float> BIOME_PARAMS[NUM_BIOMES] = {
    //OCEAN
    {
        100.0f, // SIMPLEX_LOW
        120.0f, // SIMPLEX_HIGH
    },

    //PLAINS
    {
        150.0f, // SIMPLEX_LOW
        190.0f, // SIMPLEX_HIGH
    }
};

const std::valarray<int> BIOME_INT_PARAMS[NUM_BIOMES] = {
    //OCEAN
    {
        2,   // MAIN_TERRAIN
        2,   // TREE_MIN_GRID
        5,   // TREE_MAX_GRID
        4,   // TREE_CUTOFF
        100, // TREE_DROP_CHANCE
    },
    //PLAINS
    {
        2,   // MAIN_TERRAIN
        2,   // TREE_MIN_GRID
        5,   // TREE_MAX_GRID
        4,   // TREE_CUTOFF
        0,   // TREE_DROP_CHANCE
    }
};
