#ifndef DECTREES_HPP
#define DECTREES_HPP
#include "Dec.hpp"
#include "Noise2D.hpp"

class DecTrees final : public Dec {
    public:
        DecTrees(std::mt19937* rng) : noise(rng, 0.0f, 1.0f, 1.0f) {}
        virtual ~DecTrees() {}

        void decorate(ColumnGenerator::ColumnData* col) {
        }
    private:
        Noise2D noise;
};

#endif // DECTREES_HPP
