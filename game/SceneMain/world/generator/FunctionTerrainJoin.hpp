#ifndef FUNCTIONTERRAINJOIN_HPP
#define FUNCTIONTERRAINJOIN_HPP
#include "FunctionTerrain.hpp"

class FunctionTerrainJoin : public FunctionTerrain {
    public:
        FunctionTerrainJoin(FunctionTerrain* A, FunctionTerrain* B);
        virtual ~FunctionTerrainJoin();
        virtual void fillData(int x, int z, unsigned int* data, GenParams* params) override;
    private:
        FunctionTerrain* funcA;
        FunctionTerrain* funcB;
};

#endif // FUNCTIONTERRAINJOIN_HPP
