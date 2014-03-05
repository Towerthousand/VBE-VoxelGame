#ifndef COMMONS_HPP
#define COMMONS_HPP
#include "VBE/includes.hpp"
#include "VBE/dependencies/glm/gtc/random.hpp"
#include "VBE/dependencies/glm/gtc/noise.hpp"
#include <queue>
#include <thread>
#include <mutex>
#include <random>
#include <algorithm>

constexpr int CHUNKSIZE_POW2 = 4;
constexpr int CHUNKSIZE = int(pow(2,CHUNKSIZE_POW2)); //in voxels
constexpr int CHUNKSIZE_MASK = CHUNKSIZE-1;
constexpr int WORLDSIZE = 32; //in chunks
constexpr int WORLDSIZE_MASK = WORLDSIZE-1;
constexpr int MINLIGHT = 3;
constexpr int MAXLIGHT = 16;
constexpr int TEXSIZE = 8;

#endif // COMMONS_HPP
