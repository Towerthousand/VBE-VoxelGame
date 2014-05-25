include(VBE/VBE.pro)
INCLUDEPATH += .
LIBS += -lz

#DEFINES += __DLOG
SOURCES += main.cpp \
	commons.cpp \
	SceneMain/SceneMain.cpp \
	SceneMain/DeferredContainer.cpp \
	SceneMain/DeferredLight.cpp \
	SceneMain/world/World.cpp \
	SceneMain/BlurContainer.cpp \
        SceneMain/world/Chunk.cpp \
	SceneMain/Entity.cpp \
	SceneMain/Player.cpp \
	SceneMain/Hitbox.cpp \
	SceneMain/world/DeferredCubeLight.cpp \
	SceneMain/world/Sun.cpp \
    SceneMain/Socket.cpp \
    SceneMain/NetworkManager.cpp \
    SceneMain/world/Level.cpp \
    SceneMain/PlayerBase.cpp \
    SceneMain/PlayerNetwork.cpp

HEADERS += \
	commons.hpp \
	SceneMain/SceneMain.hpp \
	SceneMain/DeferredContainer.hpp \
	SceneMain/DeferredLight.hpp \
	SceneMain/world/World.hpp \
	SceneMain/BlurContainer.hpp \
	SceneMain/world/Chunk.hpp \
        SceneMain/world/Cube.hpp \
	SceneMain/Entity.hpp \
	SceneMain/Player.hpp \
	SceneMain/Hitbox.hpp \
	SceneMain/world/DeferredCubeLight.hpp \
	SceneMain/world/generator/TaskPool.hpp \
	SceneMain/world/Sun.hpp \
    SceneMain/Socket.hpp \
    SceneMain/NetworkManager.hpp \
    SceneMain/world/Level.hpp \
    SceneMain/PlayerBase.hpp \
    SceneMain/PlayerNetwork.hpp

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

