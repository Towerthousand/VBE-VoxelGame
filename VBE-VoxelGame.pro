include(VBE/VBE.pro)
INCLUDEPATH += .
DEFINES += __DLOG
SOURCES += main.cpp \
    commons.cpp \
    SceneMain/SceneMain.cpp \
    SceneMain/Camera.cpp \
    SceneMain/PlayerCamera.cpp \
    SceneMain/DeferredModel.cpp \
    SceneMain/DeferredContainer.cpp \
	SceneMain/DeferredLight.cpp \
    SceneMain/World.cpp \
    SceneMain/BlurContainer.cpp

HEADERS += \
    commons.hpp \
    SceneMain/SceneMain.hpp \
    SceneMain/Camera.hpp \
    SceneMain/PlayerCamera.hpp \
    SceneMain/DeferredModel.hpp \
    SceneMain/DeferredContainer.hpp \
	SceneMain/DeferredLight.hpp \
    SceneMain/World.hpp \
    SceneMain/BlurContainer.hpp

OTHER_FILES += \
    data/shaders/quad.vert \
    data/shaders/standardDeferred.frag \
    data/shaders/standardDeferred.vert \
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
    data/shaders/depth.vert

