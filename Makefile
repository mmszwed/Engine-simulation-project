APP=engine_simulator
CXX=g++
CXXFLAGS=-std=c++17 -Wall -Wextra -Iinclude -Ithird_party -I/opt/homebrew/include
LDFLAGS=-L/opt/homebrew/lib
LIBS=-lglfw -lGLEW -framework OpenGL

SRC=src/cube.cpp src/lodepng.cpp src/main_file.cpp src/model.cpp src/shaderprogram.cpp src/sphere.cpp src/teapot.cpp src/torus.cpp
HEADERS=include/allmodels.h include/constants.h include/cube.h include/lodepng.h include/model.h include/myCube.h include/shaderprogram.h include/sphere.h include/teapot.h include/torus.h

$(APP): $(SRC) $(HEADERS)
	$(CXX) -o $(APP) $(SRC) $(CXXFLAGS) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(APP)
