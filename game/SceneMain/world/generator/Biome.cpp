#include "Biome.hpp"

const std::valarray<float> BIOME_PARAMS[NUM_BIOMES] = {
    //OCEAN
    {
        100.0f, // SIMPLEX_LOW
        120.0f, // SIMPLEX_HIGH
        2.0f,   // TREE_MIN_GRID
        5.0f,   // TREE_MAX_GRID
        4.0f,   // TREE_CUTOFF
        1.0f,   // TREE_DROP_CHANCE
    },

    //PLAINS
    {
        150.0f, // SIMPLEX_LOW
        190.0f, // SIMPLEX_HIGH
        2.0f,   // TREE_MIN_GRID
        5.0f,   // TREE_MAX_GRID
        4.0f,   // TREE_CUTOFF
        0.0f,   // TREE_DROP_CHANCE
    }
};
