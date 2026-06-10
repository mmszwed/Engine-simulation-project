cmake_minimum_required(VERSION 3.16)

project(EngineSimulatorR4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(engine_simulator
    src/CameraController.cpp
    src/Cylinder.cpp
    src/EngineMeshes.cpp
    src/EngineSimulator.cpp
    src/lodepng.cpp
    src/main_file.cpp
    src/shaderprogram.cpp
)

target_include_directories(engine_simulator PRIVATE
    include
    third_party
)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

target_link_libraries(engine_simulator PRIVATE
    OpenGL::GL
    GLEW::GLEW
    glfw
)
