APP=engine_simulator
CXX=g++

EXEEXT :=

COMMON_CXXFLAGS=-std=c++17 -Wall -Wextra -Iinclude -Ithird_party
SRC=src/CameraController.cpp src/Cylinder.cpp src/EngineMeshes.cpp src/EngineSimulator.cpp src/lodepng.cpp src/main_file.cpp src/shaderprogram.cpp
HEADERS=include/CameraController.h include/Cylinder.h include/EngineMeshes.h include/EngineSimulator.h include/constants.h include/lodepng.h include/shaderprogram.h

ifeq ($(OS),Windows_NT)
APP_NAME=$(APP).exe
CXXFLAGS=$(COMMON_CXXFLAGS) -Ithird_party/include
LDFLAGS=-Lthird_party/lib
ifneq ($(MINGW_PREFIX),)
CXXFLAGS += -I$(MINGW_PREFIX)/include
LDFLAGS += -L$(MINGW_PREFIX)/lib
endif
LIBS=-lglfw3 -lglew32 -lopengl32 -lgdi32
RM=cmd /C del /Q /F
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
APP_NAME=$(APP)
CXXFLAGS=$(COMMON_CXXFLAGS) -I/opt/homebrew/include
LDFLAGS=-L/opt/homebrew/lib
LIBS=-lglfw -lGLEW -framework OpenGL
RM=rm -f
else
APP_NAME=$(APP)
CXXFLAGS=$(COMMON_CXXFLAGS)
LDFLAGS=
LIBS=-lglfw -lGLEW -lGL
RM=rm -f
endif
endif

$(APP_NAME): $(SRC) $(HEADERS)
	$(CXX) -o $(APP_NAME) $(SRC) $(CXXFLAGS) $(LDFLAGS) $(LIBS)

.PHONY: clean run
run: $(APP_NAME)
	./$(APP_NAME)

clean:
	$(RM) $(APP) $(APP).exe
