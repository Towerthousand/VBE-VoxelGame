LIBS += -lGLEW -lGL -lSDL2
QMAKE_CXXFLAGS += -std=c++0x

CONFIG(debug, debug|release) {
  #DEFINES += "VBE_DETAIL"
}
