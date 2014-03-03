include(VBE/VBE.pro)
INCLUDEPATH += .
#DEFINES += __DLOG
SOURCES += main.cpp \
	commons.cpp \
	SceneMain/SceneMain.cpp \
	SceneMain/DeferredContainer.cpp \
	SceneMain/DeferredLight.cpp \
	SceneMain/world/World.cpp \
	SceneMain/BlurContainer.cpp \
	SceneMain/world/Column.cpp \
	SceneMain/world/Chunk.cpp \
	SceneMain/world/generator/Function2DConst.cpp \
	SceneMain/world/generator/Function2DSimplex.cpp \
	SceneMain/world/generator/Function3DAdd.cpp \
	SceneMain/world/generator/Function3DDiv.cpp \
	SceneMain/world/generator/Function3DSimplex.cpp \
	SceneMain/world/generator/Function3DSub.cpp \
	SceneMain/world/generator/Function3DYcoord.cpp \
	SceneMain/world/generator/FunctionTerrainHeightmap.cpp \
	SceneMain/world/generator/FunctionTerrainJoin.cpp \
	SceneMain/world/generator/FunctionTerrainOverlay.cpp \
	SceneMain/world/generator/FunctionTerrrainVolume.cpp \
	SceneMain/world/generator/ColumnGenerator.cpp \
	SceneMain/Entity.cpp \
	SceneMain/Player.cpp \
	SceneMain/Hitbox.cpp \
	SceneMain/world/DeferredCubeLight.cpp \
	SceneMain/world/Sun.cpp \
    SceneMain/world/SunCamera.cpp

HEADERS += \
	commons.hpp \
	SceneMain/SceneMain.hpp \
	SceneMain/DeferredContainer.hpp \
	SceneMain/DeferredLight.hpp \
	SceneMain/world/World.hpp \
	SceneMain/BlurContainer.hpp \
	SceneMain/world/Column.hpp \
	SceneMain/world/Chunk.hpp \
	SceneMain/world/Cube.hpp \
	SceneMain/world/generator/Function2D.hpp \
	SceneMain/world/generator/Function2DConst.hpp \
	SceneMain/world/generator/Function2DSimplex.hpp \
	SceneMain/world/generator/Function3D.hpp \
	SceneMain/world/generator/Function3DAdd.hpp \
	SceneMain/world/generator/Function3DDiv.hpp \
	SceneMain/world/generator/Function3DSimplex.hpp \
	SceneMain/world/generator/Function3DSub.hpp \
	SceneMain/world/generator/Function3DYcoord.hpp \
	SceneMain/world/generator/FunctionTerrain.hpp \
	SceneMain/world/generator/FunctionTerrainHeightmap.hpp \
	SceneMain/world/generator/FunctionTerrainJoin.hpp \
	SceneMain/world/generator/FunctionTerrainOverlay.hpp \
	SceneMain/world/generator/FunctionTerrrainVolume.hpp \
	SceneMain/world/generator/ColumnGenerator.hpp \
	SceneMain/Entity.hpp \
	SceneMain/Player.hpp \
	SceneMain/Hitbox.hpp \
	SceneMain/world/DeferredCubeLight.hpp \
	SceneMain/world/generator/TaskPool.hpp \
	SceneMain/world/Sun.hpp \
    SceneMain/world/SunCamera.hpp

OTHER_FILES += \
	data/shaders/quad.vert \
	data/shaders/light.frag \
	data/shaders/particles.frag \
	data/shaders/particles.vert \
	data/shaders/particles.geom \
	data/shaders/ambientPass.frag \
	data/shaders/blurPassVertical.frag \
	data/shaders/blurPassHoritzontal.frag \
	data/shaders/quad.frag \
	data/shaders/blurMaskPass.frag \
	data/shaders/depth.frag \
	data/shaders/depth.vert \
	data/shaders/chunkDeferred.vert \
	data/shaders/chunkDeferred.frag \
	data/shaders/occlusionQuery.vert \
	data/shaders/occlusionQuery.frag \
	data/shaders/cubeLight.frag

