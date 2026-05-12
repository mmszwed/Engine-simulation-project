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

## Build przez Makefile

Pelna instrukcja instalacji i uruchamiania jest w [docs/URUCHAMIANIE.md](docs/URUCHAMIANIE.md).

Ten sam `Makefile` obsluguje macOS oraz Windows przez MSYS2/MinGW.

### macOS

Wymagane biblioteki z Homebrew:

```sh
brew install glfw glew
make
./engine_simulator
```

### Windows

W MSYS2 wybierz jeden wariant srodowiska i zainstaluj pakiety dla niego.

UCRT64:

```sh
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-glfw mingw-w64-ucrt-x86_64-glew
mingw32-make
./engine_simulator.exe
```

MINGW64:

```sh
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-glfw mingw-w64-x86_64-glew
mingw32-make
./engine_simulator.exe
```

Na Windows uruchamiaj te komendy w terminalu MSYS2 UCRT64 albo MINGW64, nie w zwyklym PowerShellu, chyba ze masz juz `make` albo `mingw32-make`, `g++`, GLFW i GLEW w `PATH`. Jesli masz zainstalowany pakiet `make`, mozesz uzyc `make` zamiast `mingw32-make`.

## Build przez CMake

CMake zostaje jako alternatywa dla Visual Studio 2019+:

```sh
cmake -S . -B build
cmake --build build
```
