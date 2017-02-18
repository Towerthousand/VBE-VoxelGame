#ifndef TERRAINFUNCTIONS_HPP
#define TERRAINFUNCTIONS_HPP
#include "commons.hpp"
#include "Noise2D.hpp"
#include "Noise3D.hpp"
#include "Biome.hpp"

typedef double floatType; //put double here for more precision in world gen.

class Function3D { //abstract
    public:
        Function3D() {}
        virtual ~Function3D() {}
        //x,z are world coords
        virtual void fillData(int x, int z, floatType* data, const std::valarray<float>* params) = 0;
};

class Function2D : public Function3D{ //abstract
    public:
        Function2D() : Function3D() {}
        virtual ~Function2D() {}
        //x,z are world coords
        virtual floatType getValue(int x, int z, const std::valarray<float>* params) = 0;
        virtual void fillData(int x, int z, floatType* data, const std::valarray<float>* params) override final {
            floatType val = getValue(x, z, params);
            for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
                data[y] = val;
        }
};

class FunctionTerrain {//abstract
    public:
        FunctionTerrain() {}
        virtual ~FunctionTerrain() {}
        //x,z are world coords
        virtual void fillData(int x, int z, unsigned int* data, const std::valarray<float>* params) = 0;
};

class Function3DSimplex : public Function3D {
    public:
        Function3DSimplex(std::mt19937* generator, BiomeParam min, BiomeParam max, float scale)
            : noise(generator), min(min), max(max), scale(scale) {
        }
        ~Function3DSimplex() {
        }
        void fillData(int x, int z, floatType* data, const std::valarray<float>* params) override {
            for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y) {
                data[y] = (*params)[min] + ((*params)[max]-(*params)[min])*noise.octavedGet(x/scale, y/scale, z/scale, 4);
            }
        }

    private:
        Noise3D noise;

        BiomeParam min;
        BiomeParam max;
        float scale;
};

class Function3DAdd : public Function3D {
    public:
        Function3DAdd(const std::vector<Function3D*>& operands) : operands(operands) {
        }
        ~Function3DAdd() {
            for(Function3D* f : operands)
                delete f;
        }

        virtual void fillData(int x, int z, floatType* data, const std::valarray<float>* params) override {
            std::vector<floatType*> sources;
            for(unsigned int i = 1; i < operands.size(); ++i) {
                sources.push_back(new floatType[GENERATIONHEIGHT*CHUNKSIZE]);
                operands[i]->fillData(x, z, sources[i-1], params);
            }
            operands[0]->fillData(x, z, data, params);
            for(floatType* s : sources)
                for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
                    data[y] += s[y];
            for(floatType* s : sources)
                delete[] s;
        }

    private:
        std::vector<Function3D*> operands;
};

class Function3DDiv : public Function3D {
    public:
        Function3DDiv(Function3D* A, Function3D* B) : funcA(A), funcB(B) {
        }
        ~Function3DDiv() {
            delete funcA;
            delete funcB;
        }
        virtual void fillData(int x, int z, floatType* data, const std::valarray<float>* params) override {
            floatType* divisor = new floatType[GENERATIONHEIGHT*CHUNKSIZE];
            funcA->fillData(x, z, data, params);
            funcB->fillData(x, z, divisor, params);
            for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
                data[y] /= divisor[y];
            delete[] divisor;
        }

    private:
        Function3D* funcA;
        Function3D* funcB;
};

class Function3DSub : public Function3D {
    public:
        Function3DSub(Function3D* A, Function3D* B): funcA(A), funcB(B) {
        }
        ~Function3DSub(){
            delete funcA;
            delete funcB;
        }
        virtual void fillData(int x, int z, floatType* data, const std::valarray<float>* params) override {
            floatType* substract = new floatType[GENERATIONHEIGHT*CHUNKSIZE];
            funcA->fillData(x, z, data, params);
            funcB->fillData(x, z, substract, params);
            for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
                data[y] -= substract[y];
            delete[] substract;
        }

    private:
        Function3D* funcA;
        Function3D* funcB;
};

class Function3DYcoord : public Function3D {
    public:
        Function3DYcoord() {
        }
        ~Function3DYcoord() {
        }
        virtual void fillData(int x, int z, floatType* data, const std::valarray<float>* params) override {
            (void) params;
            (void) x;
            (void) z;
            for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
                data[y] = y;
        }
};

class Function3DHelix : public Function3D {
    public:
        Function3DHelix(float period, float width, float range, float offset, float yoffset, float zoffset, float tiling, float sin, float sin2) :
            period(period),
            width(width),
            range(range),
            offset(offset),
            yoffset(yoffset),
            zoffset(zoffset),
            tiling(tiling),
            sin(sin),
            sin2(sin2)
            {
        }

        ~Function3DHelix() {
        }

        virtual void fillData(int x, int z, floatType* data, const std::valarray<float>* params) override {
            (void) params;
            for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
                data[y] = helix(x, y, z);
        }

    private:
        float helix(float x, float y, float z) {
            float mc = x; //direction of the helix
            float ma = glm::mod(y-yoffset+sin*glm::sin(mc/sin2),width*tiling); //perp vector 2
            float mb = glm::mod(z-zoffset,width*tiling); //perp vector 2
            float cx = ma - width - glm::cos((mc+offset)/period) * width;
            float cz = mb - width - glm::sin((mc+offset)/period) * width;
            if(cx > 0 && cz > 0 && cx < range && cz < range) return 1000;
            if(glm::abs(ma-width+2) < 1 && glm::abs(mb-width-3) < 3) return 1000;
            return 0;
        }

        float period = 5;
        float width = 10;
        float range = 5;
        float offset = 0;
        float yoffset = 0;
        float zoffset = 0;
        float tiling = 5;
        float sin = 0;
        float sin2 = 60;
};

class Function2DConst : public Function2D {
    public:
        Function2DConst(float val) : val(val) {
        }
        ~Function2DConst() {
        }
        floatType getValue(int x, int z, const std::valarray<float>* params) override {
            (void) x;
            (void) z;
            (void) params;
            return val;
        }

    private:
        float val;
};

class Function2DSimplex : public Function2D {
    public:
        Function2DSimplex(std::mt19937* generator, BiomeParam min, BiomeParam max, float scale)
            : noise(generator), min(min), max(max), scale(scale) {
        }
        ~Function2DSimplex() {
        }
        floatType getValue(int x, int z, const std::valarray<float>* params) override {
            return (*params)[min] + ((*params)[max]-(*params)[min])*noise.octavedGet(x/scale, z/scale, 4);
        }

    private:
        Noise2D noise;

        BiomeParam min;
        BiomeParam max;
        float scale;
};

class FunctionTerrainHeightmap : public FunctionTerrain {
    public:
        FunctionTerrainHeightmap(Function2D *source, unsigned int blockID) :
            source(source), blockID(blockID) {
        }
        ~FunctionTerrainHeightmap() {
            delete source;
        }
        virtual void fillData(int x, int z, unsigned int* data, const std::valarray<float>* params) override {
            for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y)
                data[y] = source->getValue(x, z, params) < y? 0 : blockID;
        }

    private:
        Function2D* source;
        unsigned int blockID;
};

class FunctionTerrainOverlay : public FunctionTerrain {
    public:
        FunctionTerrainOverlay(FunctionTerrain *source, unsigned int overlayID, unsigned int surfaceID, unsigned int depth) :
            source(source), overlayID(overlayID), surfaceID(surfaceID), depth(depth) {
        }

        ~FunctionTerrainOverlay() {
            delete source;
        }

        virtual void fillData(int x, int z, unsigned int* data, const std::valarray<float>* params) override {
            source->fillData(x, z, data, params);
            for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE-1; ++y)
                if (data[y] == surfaceID && data[y+1] == 0)
                    for (int a = 0; a < depth; ++a) {
                        if (y-a >= 0 && data[y-a] == surfaceID)
                            data[y-a] = overlayID;
                        else
                            break;
                    }
        }

    private:
        FunctionTerrain* source;
        unsigned int overlayID;
        unsigned int surfaceID;
        int depth;
};

class FunctionTerrainVolume : public FunctionTerrain {
    public:
        FunctionTerrainVolume(Function3D *source, unsigned int blockID) :
            source(source), blockID(blockID) {
        }
        ~FunctionTerrainVolume() {
            delete source;
        }
        virtual void fillData(int x, int z, unsigned int* data, const std::valarray<float>* params) override {
            floatType* sourceData = new floatType[GENERATIONHEIGHT*CHUNKSIZE];
            source->fillData(x, z, sourceData, params);
            for(int y = 0; y < GENERATIONHEIGHT*CHUNKSIZE; ++y) {
                if(sourceData[y] <= 0.0f)
                    data[y] = 0;
                else
                    data[y] = blockID;
            }
            delete[] sourceData;
        }

    private:
        Function3D* source;
        unsigned int blockID;
};

#endif // TERRAINFUNCTION_HPP
