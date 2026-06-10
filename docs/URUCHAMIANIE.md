# Uruchamianie projektu

Projekt jest przygotowany tak, zeby dalo sie go budowac przez `Makefile` na macOS oraz Windowsie z MSYS2/MinGW.

## Sterowanie

- `A/D` - obrot kamery w lewo/prawo,
- `W/S` - obrot kamery w gore/dol,
- `Q/E` - oddalenie/przyblizenie,
- `1/2/3/4` - presety kamery,
- `strzalka gora/dol` - zwiekszenie/zmniejszenie gazu,
- `SPACE` - pauza/wznowienie,
- `R` - reset silnika,
- `L` - wlaczenie/wylaczenie lamp warsztatowych i dynamicznych cieni,
- `ESC` - wyjscie.

## macOS

### 1. Instalacja narzedzi

Zainstaluj Homebrew, jezeli jeszcze go nie masz:

```sh
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Zainstaluj biblioteki:

```sh
brew install glfw glew
```

### 2. Budowanie

W katalogu projektu:

```sh
make
```

### 3. Uruchamianie

```sh
./engine_simulator
```

### 4. Czyszczenie builda

```sh
make clean
```

## Windows przez MSYS2/MinGW

### 1. Instalacja MSYS2

Pobierz i zainstaluj MSYS2:

```text
https://www.msys2.org/
```

Po instalacji uruchom terminal **MSYS2 UCRT64** albo **MSYS2 MINGW64**. Nie uzywaj zwyklego PowerShella do pierwszej konfiguracji.

### 2. Aktualizacja MSYS2

W terminalu MSYS2:

```sh
pacman -Syu
```

Jezeli terminal poprosi o zamkniecie okna, zamknij je, otworz ponownie ten sam wariant terminala i uruchom:

```sh
pacman -Syu
```

### 3A. Wariant UCRT64

Jezeli uzywasz terminala **MSYS2 UCRT64**, zainstaluj:

```sh
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-glfw mingw-w64-ucrt-x86_64-glew
```

Budowanie:

```sh
mingw32-make
```

Uruchamianie:

```sh
./engine_simulator.exe
```

### 3B. Wariant MINGW64

Jezeli uzywasz terminala **MSYS2 MINGW64**, zainstaluj:

```sh
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-glfw mingw-w64-x86_64-glew
```

Budowanie:

```sh
mingw32-make
```

Uruchamianie:

```sh
./engine_simulator.exe
```

### 4. Czyszczenie builda

```sh
mingw32-make clean
```

## Alternatywnie: CMake

W repozytorium jest tez `CMakeLists.txt`, ale glowna sciezka projektu to `Makefile`.

Po zainstalowaniu GLFW i GLEW mozna sprobowac:

```sh
cmake -S . -B build
cmake --build build
```

## Typowe problemy

### `make: command not found`

Na Windowsie uzyj:

```sh
mingw32-make
```

albo doinstaluj pakiet `make` odpowiedni dla wybranego terminala MSYS2.

### `g++: command not found`

Nie jest zainstalowany kompilator albo uruchomiono zly terminal. Uzyj **MSYS2 UCRT64** albo **MSYS2 MINGW64** i zainstaluj pakiety z sekcji Windows.

### Brak `GL/glew.h` albo `GLFW/glfw3.h`

Nie sa zainstalowane biblioteki GLEW/GLFW dla aktywnego wariantu MSYS2. Zainstaluj pakiety z sekcji UCRT64 albo MINGW64, zgodnie z terminalem ktorego uzywasz.

### Blad linkowania z `-framework OpenGL`

To oznacza, ze uzywasz macOS-owych flag na Windowsie. Aktualny `Makefile` powinien sam rozpoznac system. Na Windowsie uruchamiaj build z terminala MSYS2.

### Program nie widzi shaderow

Uruchamiaj program z glownego katalogu projektu, czyli tam gdzie sa foldery `src/`, `include/`, `shaders/` i `textures/`.
