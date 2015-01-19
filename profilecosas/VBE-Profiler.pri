INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

LIBS += -lVBE-Profiler

# This is needed so the game is recompiled every time
# we change something in VBE-Profiler
PRE_TARGETDEPS += ../VBE-Profiler/libVBE-Profiler.a

win32 {
        CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../VBE-Profiler/release/
        CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../VBE-Profiler/debug/

        CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../VBE-Profiler/release/VBE-Profiler.lib
        CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../VBE-Profiler/debug/VBE-Profiler.lib
}

unix {
        LIBS += -L$$OUT_PWD/../VBE-Profiler/
        PRE_TARGETDEPS += $$OUT_PWD/../VBE-Profiler/libVBE-Profiler.a
}
