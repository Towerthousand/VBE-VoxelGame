#ifndef COMMONS_HPP
#define COMMONS_HPP
#include "VBE/includes.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtc/noise.hpp"
#include <queue>

#define CHUNKSIZE_POW2 4 //CHUNKSIZE must be 1 << CHUNKSIZE_POW2
#define CHUNKSIZE 16 //in voxels
#define CHUNKSIZE_MASK 15 //CHUNKSIZE -1
#define WORLDSIZE 2 //in chunks
#define WORLDSIZE_MASK 1 //WORLDWIDTH -1

#define MINLIGHT 3
#define MAXLIGHT 16

#define TEXSIZE 8

#define DEG_TO_RAD ((2*M_PI)/360.0f)

#endif // COMMONS_HPP
