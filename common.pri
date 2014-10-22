LIBS += -lGL -lSDL2
QMAKE_CXXFLAGS += -std=c++0x

CONFIG(debug, debug|release) {
  DEFINES += "__DEBUG" "__LOG"
}
