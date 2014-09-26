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
#include <bitset>
#include <unordered_map>

constexpr int CHUNKSIZE_POW2 = 4;
constexpr int CHUNKSIZE = int(pow(2,CHUNKSIZE_POW2)); //in voxels
constexpr int CHUNKSIZE_MASK = CHUNKSIZE-1;
constexpr int WORLDSIZE = 16; //in chunks
constexpr int WORLDSIZE_MASK = WORLDSIZE-1;
constexpr int MINLIGHT = 3;
constexpr int MAXLIGHT = 16;
constexpr int TEXSIZE = 8;

namespace Utils {
	template<typename T>
	inline int manhattanDistance(const glm::detail::tvec3<T>& a, const glm::detail::tvec3<T>& b) {
		return std::abs(a.x-b.x)+std::abs(a.y-b.y)+std::abs(a.z-b.z);
	}
}

#endif // COMMONS_HPP
