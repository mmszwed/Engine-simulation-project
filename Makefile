APP=engine_simulator
CXX=g++

UNAME_S := $(shell uname -s 2>/dev/null)
EXEEXT :=

COMMON_CXXFLAGS=-std=c++17 -Wall -Wextra -Iinclude -Ithird_party
SRC=src/cube.cpp src/lodepng.cpp src/main_file.cpp src/model.cpp src/shaderprogram.cpp src/sphere.cpp src/teapot.cpp src/torus.cpp
HEADERS=include/allmodels.h include/constants.h include/cube.h include/lodepng.h include/model.h include/myCube.h include/shaderprogram.h include/sphere.h include/teapot.h include/torus.h

ifeq ($(UNAME_S),Darwin)
APP_NAME=$(APP)
CXXFLAGS=$(COMMON_CXXFLAGS) -I/opt/homebrew/include
LDFLAGS=-L/opt/homebrew/lib
LIBS=-lglfw -lGLEW -framework OpenGL
RM=rm -f
else
APP_NAME=$(APP).exe
CXXFLAGS=$(COMMON_CXXFLAGS) -Ithird_party/include
LDFLAGS=-Lthird_party/lib
ifneq ($(MINGW_PREFIX),)
CXXFLAGS += -I$(MINGW_PREFIX)/include
LDFLAGS += -L$(MINGW_PREFIX)/lib
endif
LIBS=-lglfw3 -lglew32 -lopengl32 -lgdi32
RM=rm -f
endif

$(APP_NAME): $(SRC) $(HEADERS)
	$(CXX) -o $(APP_NAME) $(SRC) $(CXXFLAGS) $(LDFLAGS) $(LIBS)

.PHONY: clean run
run: $(APP_NAME)
	./$(APP_NAME)

clean:
	$(RM) $(APP) $(APP).exe
