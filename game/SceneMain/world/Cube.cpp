#include "Cube.hpp"
#include "Chunk.hpp"

inline bool shouldDraw(Cube::Type c, Cube::Type n) {
    VBE_ASSERT_SIMPLE(c != Cube::AIR);
    return (c != n && Cube::getFlag(n, Cube::TRANSPARENT));
}

void Cube::pushCubeToArray(short x, short y, short z, Cube::Type cubeID, const Chunk* c, std::vector<Cube::Vert>& renderData) {
    short texY, texX;
    std::vector<Cube::Vert> v(6);

    if(shouldDraw(cubeID, (Cube::Type) c->getCube(x, y, z+1))) { // front face
        texX = (Cube::TEXTURE_INDEXES[cubeID][0] % (Cube::ATLAS_SIZE/Cube::TEXSIZE))*Cube::TEXSIZE; // Cube::ATLAS_SIZE/Cube::TEXSIZE = number of textures/row
        texY = (Cube::TEXTURE_INDEXES[cubeID][0] / (Cube::ATLAS_SIZE/Cube::TEXSIZE))*Cube::TEXSIZE; // Cube::ATLAS_SIZE/Cube::TEXSIZE = number of textures/row

        v[0] = Cube::Vert(x+1, y+1, z+1, 0, texX              , texY              , c->calcLight(x+1, y+1, z+1, 0, 0, 1));
        v[1] = Cube::Vert(x  , y+1, z+1, 0, texX+Cube::TEXSIZE, texY              , c->calcLight(x  , y+1, z+1, 0, 0, 1));
        v[2] = Cube::Vert(x+1, y  , z+1, 0, texX              , texY+Cube::TEXSIZE, c->calcLight(x+1, y  , z+1, 0, 0, 1));

        v[3] = Cube::Vert(x  , y  , z+1, 0, texX+Cube::TEXSIZE, texY+Cube::TEXSIZE, c->calcLight(x  , y  , z+1, 0, 0, 1));
        v[4] = Cube::Vert(x+1, y  , z+1, 0, texX              , texY+Cube::TEXSIZE, c->calcLight(x+1, y  , z+1, 0, 0, 1));
        v[5] = Cube::Vert(x  , y+1, z+1, 0, texX+Cube::TEXSIZE, texY              , c->calcLight(x  , y+1, z+1, 0, 0, 1));

        if((v[1].l + v[2].l) < (v[0].l + v[3].l)) {
            v[2] = v[3];
            v[5] = v[0];
        }

        renderData.insert(renderData.end(), v.begin(), v.end());
    }
    if(shouldDraw(cubeID, (Cube::Type) c->getCube(x, y, z-1))) { // back face
        texX = (Cube::TEXTURE_INDEXES[cubeID][1] % (Cube::ATLAS_SIZE/Cube::TEXSIZE))*Cube::TEXSIZE;
        texY = (Cube::TEXTURE_INDEXES[cubeID][1] / (Cube::ATLAS_SIZE/Cube::TEXSIZE))*Cube::TEXSIZE;

        v[0] = Cube::Vert(x+1, y  , z, 1, texX              , texY+Cube::TEXSIZE, c->calcLight(x+1, y  , z, 0, 0, -1));
        v[1] = Cube::Vert(x  , y+1, z, 1, texX+Cube::TEXSIZE, texY              , c->calcLight(x  , y+1, z, 0, 0, -1));
        v[2] = Cube::Vert(x+1, y+1, z, 1, texX              , texY              , c->calcLight(x+1, y+1, z, 0, 0, -1));

        v[3] = Cube::Vert(x  , y  , z, 1, texX+Cube::TEXSIZE, texY+Cube::TEXSIZE, c->calcLight(x  , y  , z, 0, 0, -1));
        v[4] = Cube::Vert(x  , y+1, z, 1, texX+Cube::TEXSIZE, texY              , c->calcLight(x  , y+1, z, 0, 0, -1));
        v[5] = Cube::Vert(x+1, y  , z, 1, texX              , texY+Cube::TEXSIZE, c->calcLight(x+1, y  , z, 0, 0, -1));

        if((v[0].l + v[1].l) < (v[2].l + v[3].l)) {
            v[0] = v[3];
            v[4] = v[2];
        }

        renderData.insert(renderData.end(), v.begin(), v.end());
    }
    if(shouldDraw(cubeID, (Cube::Type) c->getCube(x+1, y, z))) { // left face
        texX = (Cube::TEXTURE_INDEXES[cubeID][2] % (Cube::ATLAS_SIZE/Cube::TEXSIZE))*Cube::TEXSIZE;
        texY = (Cube::TEXTURE_INDEXES[cubeID][2] / (Cube::ATLAS_SIZE/Cube::TEXSIZE))*Cube::TEXSIZE;

        v[0] = Cube::Vert(x+1, y  , z+1, 2, texX              , texY+Cube::TEXSIZE, c->calcLight(x+1, y  , z+1, 1, 0, 0));
        v[1] = Cube::Vert(x+1, y  , z  , 2, texX+Cube::TEXSIZE, texY+Cube::TEXSIZE, c->calcLight(x+1, y  , z  , 1, 0, 0));
        v[2] = Cube::Vert(x+1, y+1, z+1, 2, texX              , texY              , c->calcLight(x+1, y+1, z+1, 1, 0, 0));

        v[3] = Cube::Vert(x+1, y  , z  , 2, texX+Cube::TEXSIZE, texY+Cube::TEXSIZE, c->calcLight(x+1, y  , z  , 1, 0, 0));
        v[4] = Cube::Vert(x+1, y+1, z  , 2, texX+Cube::TEXSIZE, texY              , c->calcLight(x+1, y+1, z  , 1, 0, 0));
        v[5] = Cube::Vert(x+1, y+1, z+1, 2, texX              , texY              , c->calcLight(x+1, y+1, z+1, 1, 0, 0));

        if((v[1].l + v[2].l) < (v[0].l + v[4].l)) {
            v[1] = v[4];
            v[5] = v[0];
        }

        renderData.insert(renderData.end(), v.begin(), v.end());
    }
    if(shouldDraw(cubeID, (Cube::Type) c->getCube(x-1, y, z))) { // right face
        texX = (Cube::TEXTURE_INDEXES[cubeID][3] % (Cube::ATLAS_SIZE/Cube::TEXSIZE))*Cube::TEXSIZE;
        texY = (Cube::TEXTURE_INDEXES[cubeID][3] / (Cube::ATLAS_SIZE/Cube::TEXSIZE))*Cube::TEXSIZE;

        v[0] = Cube::Vert(x  , y  , z+1, 3, texX              , texY+Cube::TEXSIZE, c->calcLight(x  , y  , z+1, -1, 0, 0));
        v[1] = Cube::Vert(x  , y+1, z+1, 3, texX              , texY              , c->calcLight(x  , y+1, z+1, -1, 0, 0));
        v[2] = Cube::Vert(x  , y  , z  , 3, texX+Cube::TEXSIZE, texY+Cube::TEXSIZE, c->calcLight(x  , y  , z  , -1, 0, 0));

        v[3] = Cube::Vert(x  , y+1, z+1, 3, texX              , texY              , c->calcLight(x  , y+1, z+1, -1, 0, 0));
        v[4] = Cube::Vert(x  , y+1, z  , 3, texX+Cube::TEXSIZE, texY              , c->calcLight(x  , y+1, z  , -1, 0, 0));
        v[5] = Cube::Vert(x  , y  , z  , 3, texX+Cube::TEXSIZE, texY+Cube::TEXSIZE, c->calcLight(x  , y  , z  , -1, 0, 0));

        if((v[1].l + v[2].l) < (v[0].l + v[4].l)) {
            v[1] = v[4];
            v[5] = v[0];
        }

        renderData.insert(renderData.end(), v.begin(), v.end());
    }
    if(shouldDraw(cubeID, (Cube::Type) c->getCube(x, y-1, z))) { // bottom face
        texX = (Cube::TEXTURE_INDEXES[cubeID][4] % (Cube::ATLAS_SIZE/Cube::TEXSIZE))*Cube::TEXSIZE;
        texY = (Cube::TEXTURE_INDEXES[cubeID][4] / (Cube::ATLAS_SIZE/Cube::TEXSIZE))*Cube::TEXSIZE;

        v[0] = Cube::Vert(x+1, y, z  , 4, texX+Cube::TEXSIZE, texY              , c->calcLight(x+1, y, z  , 0, -1, 0));
        v[1] = Cube::Vert(x  , y, z+1, 4, texX              , texY+Cube::TEXSIZE, c->calcLight(x  , y, z+1, 0, -1, 0));
        v[2] = Cube::Vert(x  , y, z  , 4, texX              , texY              , c->calcLight(x  , y, z  , 0, -1, 0));

        v[3] = Cube::Vert(x+1, y, z  , 4, texX+Cube::TEXSIZE, texY              , c->calcLight(x+1, y, z  , 0, -1, 0));
        v[4] = Cube::Vert(x+1, y, z+1, 4, texX+Cube::TEXSIZE, texY+Cube::TEXSIZE, c->calcLight(x+1, y, z+1, 0, -1, 0));
        v[5] = Cube::Vert(x  , y, z+1, 4, texX              , texY+Cube::TEXSIZE, c->calcLight(x  , y, z+1, 0, -1, 0));

        if((v[0].l + v[1].l) < (v[2].l + v[4].l)) {
            v[1] = v[4];
            v[3] = v[2];
        }

        renderData.insert(renderData.end(), v.begin(), v.end());
    }
    if(shouldDraw(cubeID, (Cube::Type) c->getCube(x, y+1, z))) { // bottom face
        texX = (Cube::TEXTURE_INDEXES[cubeID][5] % (Cube::ATLAS_SIZE/Cube::TEXSIZE))*Cube::TEXSIZE;
        texY = (Cube::TEXTURE_INDEXES[cubeID][5] / (Cube::ATLAS_SIZE/Cube::TEXSIZE))*Cube::TEXSIZE;

        v[0] = Cube::Vert(x+1, y+1, z  , 5, texX+Cube::TEXSIZE, texY              , c->calcLight(x+1, y+1, z  , 0, 1, 0));
        v[1] = Cube::Vert(x  , y+1, z  , 5, texX              , texY              , c->calcLight(x  , y+1, z  , 0, 1, 0));
        v[2] = Cube::Vert(x  , y+1, z+1, 5, texX              , texY+Cube::TEXSIZE, c->calcLight(x  , y+1, z+1, 0, 1, 0));

        v[3] = Cube::Vert(x+1, y+1, z  , 5, texX+Cube::TEXSIZE, texY              , c->calcLight(x+1, y+1, z  , 0, 1, 0));
        v[4] = Cube::Vert(x  , y+1, z+1, 5, texX              , texY+Cube::TEXSIZE, c->calcLight(x  , y+1, z+1, 0, 1, 0));
        v[5] = Cube::Vert(x+1, y+1, z+1, 5, texX+Cube::TEXSIZE, texY+Cube::TEXSIZE, c->calcLight(x+1, y+1, z+1, 0, 1, 0));

        if((v[0].l + v[2].l) < (v[1].l + v[5].l)) {
            v[2] = v[5];
            v[3] = v[1];
        }

        renderData.insert(renderData.end(), v.begin(), v.end());
    }
}
