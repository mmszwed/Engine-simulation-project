# Symulator pracy silnika czterosuwowego R4

Projekt z grafiki komputerowej w C++/OpenGL/GLM. Repozytorium bazuje na paczce laboratoryjnej, ale jest uporzadkowane pod docelowy symulator silnika.

## Struktura

- `src/` - pliki `.cpp` aplikacji i startera laboratoryjnego.
- `include/` - naglowki.
- `shaders/` - shadery GLSL.
- `textures/` - tekstury uzywane w scenie.
- `third_party/glm/` - lokalna kopia GLM.
- `docs/` - PRD, plan zadan, zasady projektu i materialy z zajec.

## Aktualny stan

Starter OpenGL jest zachowany jako punkt wyjscia. Kolejny etap to dodanie pierwszych modulow projektu: `CameraController`, `EngineSimulator`, `Cylinder` i proceduralnych meshy silnika.

## Build

Makefile pochodzi ze startera labowego i jest ustawiony pod macOS/Homebrew:

```sh
make
```

Na Windows preferowany jest CMake z zainstalowanymi bibliotekami GLFW i GLEW:

```sh
cmake -S . -B build
cmake --build build
```
