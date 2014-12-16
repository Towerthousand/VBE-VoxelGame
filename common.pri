LIBS += -lGLEW -lGL -lSDL2
QMAKE_CXXFLAGS += -std=c++0x -fno-exceptions

CONFIG(debug, debug|release) {
  #DEFINES += "VBE_DETAIL"
}
