#ifndef BIOMEFUNCTIONS_HPP
#define BIOMEFUNCTIONS_HPP
#include "commons.hpp"

class BiomeSet final {
    public:
        BiomeSet(std::set<int> b, bool reverse=false) : b(b), reverse(reverse) {}
        ~BiomeSet() {}

        bool test(int biome) const {
            return reverse? !b.count(biome) : b.count(biome);
        }
    private:
        const std::set<int> b;
        const bool reverse = false;
};

class FunctionBiome { //abstract
    public:
        FunctionBiome(std::mt19937* generator) {
            randomOffset = (*generator)();
        }
        virtual ~FunctionBiome() {
        }

        virtual std::vector<int> getBiomeData(int px, int pz, int sx, int sz) const = 0;

    protected:
        int randomOffset = 0;
        const long multiplier = 0x5DEECE66Dl;
        const long addend = 0xBl;
        const long mask = (1l << 48) - 1;

        int randForPos(int max, int px, int pz, int n) const {
            long seed = px*341651197+pz*84719323+n*517375 + randomOffset;

            seed = (seed ^ multiplier) & mask;
            seed = (seed * multiplier + addend) & mask;
            if ((max & -max) == max) { // i.e., max is a power of 2
                seed = ((unsigned long) seed) >> (48-31);
                return (int)((max * seed) >> 31);
            }

            return (int)(((seed % max)+max)%max);
        }
};

class BiomeConst final : public FunctionBiome {
    public:
        BiomeConst(std::mt19937* generator, int biome) : FunctionBiome(generator), biome(biome) {
        }

        virtual ~BiomeConst() {
        }

        std::vector<int> getBiomeData(int px, int pz, int sx, int sz) const override {
            (void) px;
            (void) pz;
            return std::vector<int>(sx*sz, biome);
        }
    private:
        const int biome = 0;
};

class BiomeCombine final : public FunctionBiome {
    public:
        BiomeCombine(std::mt19937* generator, FunctionBiome* baseA, FunctionBiome* baseB, BiomeSet setA, BiomeSet setB, int replace=-1)
            : FunctionBiome(generator), baseA(baseA), baseB(baseB), setA(setA), setB(setB), replace(replace) {
        }
        virtual ~BiomeCombine() {
        }

        std::vector<int> getBiomeData(int px, int pz, int sx, int sz) const override {
            std::vector<int> dataA = baseA->getBiomeData(px, pz, sx, sz);
            std::vector<int> dataB = baseB->getBiomeData(px, pz, sx, sz);

            for(int i = 0; i < sx*sz; i++) {
                if(setA.test(dataA[i]) && setB.test(dataB[i])) {
                    if(replace == -1) dataA[i] = dataB[i];
                    else dataA[i] = replace;
                }
            }

            return dataA;
        }
    private:
        const FunctionBiome* baseA = nullptr;
        const FunctionBiome* baseB = nullptr;
        const BiomeSet setA;
        const BiomeSet setB;
        const int replace = -1;
};

class BiomeIsland final : public FunctionBiome {
    public:
        BiomeIsland(std::mt19937* generator, FunctionBiome* base) : FunctionBiome(generator), base(base) {
        }
        virtual ~BiomeIsland() {
        }

        std::vector<int> getBiomeData(int px, int pz, int sx, int sz) const override {
            int px2 = px - 1;
            int pz2 = pz - 1;
            int sx2 = sx + 2;
            int sz2 = sz + 2;
            std::vector<int> baseData = base->getBiomeData(px2, pz2, sx2, sz2);
            std::vector<int> data(sx * sz);

            for (int z = 0; z < sz; ++z) {
                for (int x = 0; x < sx; ++x) {
                    int topLeft     = baseData[x + 0 + (z + 0) * sx2];
                    int topRight    = baseData[x + 2 + (z + 0) * sx2];
                    int bottomLeft  = baseData[x + 0 + (z + 2) * sx2];
                    int bottomRight = baseData[x + 2 + (z + 2) * sx2];
                    int center      = baseData[x + 1 + (z + 1) * sx2];

                    //ocean surrounded by some other biome
                    if (center == 0 && (topLeft != 0 || topRight != 0 || bottomLeft != 0 || bottomRight != 0)) {
                        int max = 1;
                        int r = 1;

                        //Choose randomly from the 4 values one that's not 0
                        if (    topLeft != 0 && randForPos(max++, x + px, z + pz, 0) == 0) r = topLeft;
                        if (   topRight != 0 && randForPos(max++, x + px, z + pz, 1) == 0) r = topRight;
                        if ( bottomLeft != 0 && randForPos(max++, x + px, z + pz, 2) == 0) r = bottomLeft;
                        if (bottomRight != 0 && randForPos(max++, x + px, z + pz, 3) == 0) r = bottomRight;

                        if (randForPos(3, x + px, z + pz, 4) == 0)
                            data[x + z * sx] = r;
                        else
                            data[x + z * sx] = 0;
                    }
                    //some other biome by the ocean
                    else if (center > 0 && (topLeft == 0 || topRight == 0 || bottomLeft == 0 || bottomRight == 0)) {
                        //if rand()%5 and id not icePlains
                        if (randForPos(5, x + px, z + pz, 5) == 0)
                                data[x + z * sx] = 0;
                        else
                            data[x + z * sx] = center;
                    }
                    //inland biome or full ocean
                    else
                        data[x + z * sx] = center;
                }
            }

            return data;
        }
    private:
        const FunctionBiome* base = nullptr;
};

class BiomeOutline final : public FunctionBiome {
    public:
        BiomeOutline(std::mt19937* generator, FunctionBiome* base, BiomeSet biomes, BiomeSet nextTo, int outlineBiome, bool outer = false) :
            FunctionBiome(generator), base(base), biomes(biomes), nextTo(nextTo), outlineBiome(outlineBiome), outer(outer) {
            VBE_ASSERT_SIMPLE(base != nullptr);
        }

        virtual ~BiomeOutline() {
        }

        std::vector<int> getBiomeData(int px, int pz, int sx, int sz) const override {
            std::vector<int> baseData = base->getBiomeData(px - 1, pz - 1, sx + 2, sz + 2);
            std::vector<int> result(sx * sz);

            for (int z = 0; z < sz; ++z) {
                for (int x = 0; x < sx; ++x) {
                    int center = baseData[x + 1 + (z + 1) * (sx + 2)];
                    int bottom = baseData[x + 1 + (z + 1 - 1) * (sx + 2)];
                    int right = baseData[x + 1 + 1 + (z + 1) * (sx + 2)];
                    int left = baseData[x + 1 - 1 + (z + 1) * (sx + 2)];
                    int top = baseData[x + 1 + (z + 1 + 1) * (sx + 2)];

                    if(outer) {
                        if (!biomes.test(center)
                            && (biomes.test(bottom) || biomes.test(right) || biomes.test(left) || biomes.test(top))
                            && nextTo.test(bottom) && nextTo.test(right) && nextTo.test(left) && nextTo.test(top))
                            result[x+z*sx] = outlineBiome;
                        else
                            result[x+z*sx] = center;
                    }
                    else {
                        if (biomes.test(center)
                            && (bottom != center || right != center || left != center || top != center)
                            && nextTo.test(bottom) && nextTo.test(right) && nextTo.test(left) && nextTo.test(top))
                            result[x+z*sx] = outlineBiome;
                        else
                            result[x+z*sx] = center;
                    }
                }
            }
            return result;
        }

    private:
        const FunctionBiome* base = nullptr;
        const BiomeSet biomes;
        const BiomeSet nextTo;
        const int outlineBiome = 0;
        const bool outer = false;
};

class BiomeReplace final : public FunctionBiome {
    public:
        BiomeReplace(std::mt19937* generator, FunctionBiome* base, BiomeSet from, std::vector<std::pair<int, int>> to, bool noEdges = false) :
            FunctionBiome(generator), base(base), from(from), to(to), noEdges(noEdges) {
            VBE_ASSERT_SIMPLE(base != nullptr);
            VBE_ASSERT_SIMPLE(!to.empty());
        }
        BiomeReplace(std::mt19937* generator, FunctionBiome* base, BiomeSet from, int to, int prob, bool noEdges = false) :
            FunctionBiome(generator), base(base), from(from), to({{to, 1}, {-1, prob-1}}), noEdges(noEdges) {
            VBE_ASSERT_SIMPLE(prob > 1);
            VBE_ASSERT_SIMPLE(base != nullptr);
        }

        virtual ~BiomeReplace() {
        }

        std::vector<int> getBiomeData(int px, int pz, int sx, int sz) const override {
            int s = 0;
            for(const std::pair<int, int>& t : to)
                s += t.second;

            if(noEdges) {
                std::vector<int> baseData = base->getBiomeData(px - 1, pz - 1, sx + 2, sz + 2);
                std::vector<int> result(sx * sz);

                for (int z = 0; z < sz; ++z) {
                    for (int x = 0; x < sx; ++x) {
                        int old = baseData[x + 1 + (z + 1) * (sx + 2)];
                        int val = old;
                        if(from.test(val)) {
                            int r = randForPos(s, px + x, pz + z, 0);
                            for(const std::pair<int, int>& p : to) {
                                r -= p.second;
                                if(r < 0) {
                                    if(p.first != -1)
                                        val = p.first;
                                    break;
                                }
                            }
                        }

                        if (val == old)
                            result[x + z * sx] = old;
                        else {
                            int top = baseData[x + 1 + (z + 1 - 1) * (sx + 2)];
                            int right = baseData[x + 1 + 1 + (z + 1) * (sx + 2)];
                            int left = baseData[x + 1 - 1 + (z + 1) * (sx + 2)];
                            int down = baseData[x + 1 + (z + 1 + 1) * (sx + 2)];

                            if (top == old && right == old && left == old && down == old)
                                result[x + z * sx] = val;
                            else
                                result[x + z * sx] = old;
                        }
                    }
                }
                return result;
            }

            std::vector<int> result = base->getBiomeData(px, pz, sx, sz);
            for (int z = 0; z < sz; ++z)
                for (int x = 0; x < sx; ++x)
                    if(from.test(result[x + z * sx])) {
                        int r = randForPos(s, px + x, pz + z, 0);
                        for(const std::pair<int, int>& p : to) {
                            r -= p.second;
                            if(r < 0) {
                                if(p.first != -1)
                                    result[x + z * sx] = p.first;
                                break;
                            }
                        }
                    }

            return result ;
        }

    private:
        const FunctionBiome* base = nullptr;
        const BiomeSet from;
        const std::vector<std::pair<int, int>> to;
        const bool noEdges = false;
};

class BiomeSmooth final : public FunctionBiome {
    public:
        BiomeSmooth(std::mt19937* generator, FunctionBiome* base) : FunctionBiome(generator), base(base) {
        }
        virtual ~BiomeSmooth() {
        }

        std::vector<int> getBiomeData(int px, int pz, int sx, int sz) const override {
            int px2 = px - 1;
            int pz2 = pz - 1;
            int sx2 = sx + 2;
            int sz2 = sz + 2;
            std::vector<int> baseData = base->getBiomeData(px2, pz2, sx2, sz2);
            std::vector<int> data(sx * sz);

            for (int z = 0; z < sz; ++z) {
                for (int x = 0; x < sx; ++x) {
                    int left = baseData[x + 0 + (z + 1) * sx2];
                    int right = baseData[x + 2 + (z + 1) * sx2];
                    int top = baseData[x + 1 + (z + 0) * sx2];
                    int bottom = baseData[x + 1 + (z + 2) * sx2];
                    int center = baseData[x + 1 + (z + 1) * sx2];

                    if (left == right && top == bottom) {
                        if (randForPos(2, x + px, z + pz, 0) == 0)
                            center = left;
                        else
                            center = top;
                    }
                    else {
                        if (left == right)
                            center = left;
                        if (top == bottom)
                            center = top;
                    }
                    data[x + z * sx] = center;
                }
            }
            return data;
        }

    private:
        const FunctionBiome* base = nullptr;
};

class BiomeZoom final : public FunctionBiome {
    public:
        BiomeZoom(std::mt19937* generator, FunctionBiome* base, bool fuzzy=false) : FunctionBiome(generator), base(base), fuzzy(fuzzy) {
            VBE_ASSERT_SIMPLE(base != nullptr);
        }

        virtual ~BiomeZoom() {
        }

        std::vector<int> getBiomeData(int px, int pz, int sx, int sz) const override {
            int baseX = px >> 1;
            int baseZ = pz >> 1;
            int baseSizeX = (sx >> 1) + 3;
            int baseSizeZ = (sz >> 1) + 3;
            int zoomedSizeX = baseSizeX * 2;
            int zoomedSizeZ = baseSizeZ * 2;
            std::vector<int>  baseData = base->getBiomeData(baseX, baseZ, baseSizeX, baseSizeZ);
            std::vector<int>  zoomedData = std::vector<int>(zoomedSizeX * zoomedSizeZ);

            for (int z = 0; z < baseSizeZ - 1; ++z) {
                int currZoomedZ = (z << 1) * zoomedSizeX;
                int topleft = baseData[0 + (z + 0) * baseSizeX]; //baseData[z][0]
                int bottomleft = baseData[0 + (z + 1) * baseSizeX]; //baseData[z+1][0]

                for (int x = 0; x < baseSizeX - 1; ++x) {
                    int topright = baseData[x + 1 + (z + 0) * baseSizeX]; //baseData[z][x+1]
                    int bottomright = baseData[x + 1 + (z + 1) * baseSizeX]; //baseData[z+1][x+1]
                    // Set topleft corner
                    zoomedData[currZoomedZ] = topleft;
                    // Set bottomleft corner
                    zoomedData[currZoomedZ++ + zoomedSizeX] = choose(topleft, bottomleft, x + baseX, z + baseZ, 0);
                    // Set topright corner
                    zoomedData[currZoomedZ] = choose(topleft, topright, x + baseX, z + baseZ, 1);
                    // Set bottomRight corner
                    if(fuzzy)
                        zoomedData[currZoomedZ++ + zoomedSizeX] = choose(topleft, topright, bottomleft, bottomright, x + baseX, z + baseZ, 2);
                    else
                        zoomedData[currZoomedZ++ + zoomedSizeX] = modeOrRandom(topleft, topright, bottomleft, bottomright, x + baseX, z + baseZ, 2);
                    topleft = topright;
                    bottomleft = bottomright;
                }
            }

            std::vector<int>  result = std::vector<int>(sx * sz);

            for (int z = 0; z < sz; ++z) {
                std::vector<int>::const_iterator start = zoomedData.begin() + (z + (pz & 1)) * zoomedSizeX + (px & 1);
                result.insert(
                    result.begin() + (z*sx),
                    start,
                    start+sx
                );
            }

            return result;
        }

    private:
        int choose(int a, int b, int x, int z, int n) const {
            return randForPos(2, x, z, n) == 0 ? a : b;
        }

        int choose(int a, int b, int c, int d, int x, int z, int n) const {
            int r = randForPos(4, x, z, n);
            return r == 0 ? a : (r == 1 ? b : (r == 2 ? c : d));
        }

        int modeOrRandom(int a, int b, int c, int d, int x, int z, int n) const {
            if (b == c && c == d) return b;
            else if (a == b && a == c) return a;
            else if (a == b && a == d) return a;
            else if (a == c && a == d) return a;
            else if (a == b && c != d) return a;
            else if (a == c && b != d) return a;
            else if (a == d && b != c) return a;
            else if (b == a && c != d) return b;
            else if (b == c && a != d) return b;
            else if (b == d && a != c) return b;
            else if (c == a && b != d) return c;
            else if (c == b && a != d) return c;
            else if (c == d && a != b) return c;
            else if (d == a && b != c) return c;
            else if (d == b && a != c) return c;
            else if (d == c && a != b) return c;
            else {
                int r = this->randForPos(4, x, z, n);
                return r == 0 ? a : (r == 1 ? b : (r == 2 ? c : d));
            }
        }

        const FunctionBiome* base = nullptr;
        const bool fuzzy = false;
};

#endif // BIOMEFUNCTIONS_HPP
