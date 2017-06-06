#ifndef BIOME_HPP
#define BIOME_HPP
#include <valarray>

enum Biome {
    OCEAN = 0,
    PLAINS,
    NUM_BIOMES
};

enum BiomeParam {
    SIMPLEX_LOW = 0,
    SIMPLEX_HIGH,
    LOW_SURFACE_DEPTH,
    SURFACE_DEPTH,
    NUM_BIOME_PARAMS
};

enum BiomeIntParam {
    MAIN_TERRAIN = 0,
    LOW_SURFACE,
    SURFACE,
    TREE_MIN_GRID,
    TREE_MAX_GRID,
    TREE_CUTOFF,
    TREE_DROP_CHANCE,
    SEA_LEVEL,
};

extern const std::valarray<float> BIOME_PARAMS[NUM_BIOMES];
extern const std::valarray<int> BIOME_INT_PARAMS[NUM_BIOMES];

#endif // BIOME_HPP
