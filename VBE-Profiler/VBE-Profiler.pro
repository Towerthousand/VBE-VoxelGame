QT       -= core gui

TARGET = VBE-Profiler
TEMPLATE = lib
CONFIG += staticlib

unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += include src

include(../VBE/VBE.pri)
include(../VBE-Scenegraph/VBE-Scenegraph.pri)
include(../common.pri)

OTHER_FILES += \
    VBE-Profiler.pri

HEADERS += \
    include/VBE-Profiler/profiler.hpp \
    include/VBE-Profiler/VBE-Profiler.hpp \
    include/VBE-Profiler/profiler/imgui.hpp \
    include/VBE-Profiler/profiler/Profiler.hpp \
    include/VBE-Profiler/profiler/imconfig.hpp \
    include/VBE-Profiler/profiler/stb_textedit.hpp

SOURCES += \
    src/VBE-Profiler/profiler/Profiler.cpp \
    src/VBE-Profiler/profiler/imgui.cpp
