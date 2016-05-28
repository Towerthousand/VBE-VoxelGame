#ifndef COMMONS_HPP
#define COMMONS_HPP
#include <VBE/VBE.hpp>
#include <VBE-Scenegraph/VBE-Scenegraph.hpp>
#include <VBE-Profiler/VBE-Profiler.hpp>
#include <VBE/dependencies/glm/gtc/random.hpp>
#include <VBE/dependencies/glm/gtc/noise.hpp>
#include <queue>
#include <thread>
#include <mutex>
#include <random>
#include <algorithm>
#include <bitset>
#include <unordered_set>

constexpr int CHUNKSIZE_POW2 = 4;
constexpr int CHUNKSIZE = 16; //in voxels
constexpr int CHUNKSIZE_MASK = CHUNKSIZE-1;
constexpr int WORLDSIZE = 512/CHUNKSIZE;
constexpr int GENERATIONHEIGHT = 256/CHUNKSIZE;
constexpr int WORLDSIZE_MASK = WORLDSIZE-1;
constexpr int TEXSIZE = 8;

namespace Utils {
    template<typename T>
    inline int manhattanDistance(const glm::tvec3<T>& a, const glm::tvec3<T>& b) {
        return std::abs(a.x-b.x)+std::abs(a.y-b.y)+std::abs(a.z-b.z);
    }
    template<typename T>
    std::string toString(T arg) {
      std::ostringstream temp;
      temp << arg;
      return temp.str();
    }
}

#endif // COMMONS_HPP
