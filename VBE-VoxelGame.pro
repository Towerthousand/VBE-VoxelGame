include(VBE/VBE.pro)
INCLUDEPATH += .
DEFINES += __DLOG
SOURCES += main.cpp \
    commons.cpp \
    SceneMain/SceneMain.cpp \
    SceneMain/Camera.cpp \
    SceneMain/PlayerCamera.cpp \
    SceneMain/DeferredContainer.cpp \
	SceneMain/DeferredLight.cpp \
	SceneMain/world/World.cpp \
    SceneMain/BlurContainer.cpp \
	SceneMain/world/Column.cpp \
	SceneMain/world/Chunk.cpp

HEADERS += \
    commons.hpp \
    SceneMain/SceneMain.hpp \
    SceneMain/Camera.hpp \
    SceneMain/PlayerCamera.hpp \
    SceneMain/DeferredContainer.hpp \
	SceneMain/DeferredLight.hpp \
	SceneMain/world/World.hpp \
    SceneMain/BlurContainer.hpp \
	SceneMain/world/Column.hpp \
	SceneMain/world/Chunk.hpp \
    SceneMain/world/Cube.hpp

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
    data/shaders/chunkDeferred.frag

