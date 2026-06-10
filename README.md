# Symulator pracy silnika czterosuwowego R4

Interaktywny projekt z grafiki komputerowej przedstawiajacy prace czterosuwowego
silnika benzynowego R4 w przekroju. Program zostal napisany w C++17 z uzyciem
OpenGL 3.3 Core, GLFW, GLEW oraz GLM.

Scena pokazuje miedzy innymi tloki, korbowody, wal korbowy, dwa walki rozrzadu,
zawory, pompowtryskiwacze, lancuch rozrzadu, dolot, wydech i efekty kolejnych
suwow pracy. Silnik znajduje sie na stanowisku warsztatowym w garazu.

## Struktura

- `src/` - implementacja aplikacji, silnika, kamery, meshy i shaderow.
- `include/` - publiczne interfejsy modulow.
- `shaders/` - shader materialow oraz shader mapy cienia.
- `textures/` - cztery tekstury faktycznie wykorzystywane w scenie.
- `third_party/glm/` - lokalna kopia GLM.
- `docs/` - PRD, plan zadan, zasady projektu i materialy z zajec.

## Architektura

- `main_file.cpp` - tworzenie okna i kontekstu OpenGL, glowna petla oraz dwa
  przebiegi renderowania.
- `CameraController` - kamera orbitujaca, zoom i cztery presety widoku.
- `EngineSimulator` - stan animacji, RPM, gaz, oswietlenie, otoczenie oraz
  koordynacja wszystkich zespolow silnika.
- `Cylinder` - logika pojedynczego cylindra: suw, tlok, korbowod, zawory,
  wtrysk i efekty wewnatrz komory.
- `EngineMesh` - generowanie i przechowywanie proceduralnych meshy z VAO/VBO.
- `ShaderProgram` - wczytywanie, kompilacja, linkowanie i obsluga shaderow.
- `lodepng` - wczytywanie tekstur PNG.

Wszystkie modele silnika i otoczenia sa budowane proceduralnie. Renderowanie
korzysta z `glDrawArrays`, a transformacje modelu, widoku i projekcji sa
wykonywane przez GLM.

## Mechanika R4

Pelny cykl pracy cylindra trwa 720 stopni obrotu walu:

1. ssanie,
2. sprezanie,
3. praca,
4. wydech.

Tlokami 1 i 4 oraz 2 i 3 steruja wspolne pary wykorbien przesuniete o 180
stopni. Przesuniecia cyklu cylindrow realizuja typowa kolejnosc zaplonu
**1-3-4-2**. Walki rozrzadu obracaja sie z polowa predkosci walu korbowego,
a ruch zaworow, wtrysku, lancucha i efektow przeplywu jest synchronizowany
z aktualnym katem walu.

## Rendering i oswietlenie

Glowny shader wykorzystuje model Blinn-Phong z teksturowaniem, swiatlem
kierunkowym oraz dwiema lampami warsztatowymi. Klawisz `L` przelacza lampy
i ich wplyw na scene.

Dynamiczne cienie sa realizowane przez shadow mapping:

1. scena jest renderowana z pozycji lampy do mapy glebi 2048x2048,
2. w przebiegu glownym pozycja fragmentu jest porownywana z mapa glebi,
3. filtrowanie PCF 5x5 wygladza krawedzie cienia,
4. bias zalezny od normalnej ogranicza efekt shadow acne.

Shader materialow pracuje w przestrzeni widoku. Pozycje i kierunki swiatel
sa przeliczane macierza kamery, dlatego oswietlenie pozostaje nieruchome
w swiecie podczas obracania widoku.

## Sterowanie

Program startuje w pauzie, z minimalnym gazem. Nacisnij `SPACE`, zeby rozpoczac animacje.

- `A/D` - obrot kamery w lewo/prawo,
- `W/S` - obrot kamery w gore/dol,
- `Q/E` - oddalenie/przyblizenie,
- `1/2/3/4` - presety kamery,
- `strzalka gora/dol` - zwiekszenie/zmniejszenie gazu,
- `L` - wlaczenie/wylaczenie lamp warsztatowych i dynamicznych cieni,
- `SPACE` - pauza/wznowienie,
- `R` - reset animacji, RPM i gazu oraz powrot do pauzy,
- `ESC` - wyjscie.

## Status testow

- czysty build `mingw32-make clean && mingw32-make` przechodzi bez ostrzezen,
- aplikacja uruchamia sie bez bledow kompilacji i linkowania shaderow,
- wszystkie tekstury i mapa cienia sa odnajdywane przy starcie z katalogu repo,
- sterowanie `A/D`, `W/S`, `Q/E`, `1-4`, strzalki, `L`, `SPACE`, `R` i `ESC`
  zostalo sprawdzone recznie i dziala poprawnie,
- kod nie zawiera zakazanych polecen starego OpenGL wymienionych w wymaganiach.

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

## Dokumentacja Doxygen

Repozytorium zawiera plik `Doxyfile` oraz komentarze dokumentacyjne klas,
metod i typow projektu. Po zainstalowaniu Doxygen i Graphviz dokumentacje
HTML mozna wygenerowac poleceniem:

```sh
mingw32-make docs
```

Na macOS lub Linuxie mozna uzyc:

```sh
make docs
```

Strona glowna zostanie utworzona jako:

```text
docs/doxygen/html/index.html
```
