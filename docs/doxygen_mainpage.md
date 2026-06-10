# Symulator pracy silnika czterosuwowego R4

## Cel projektu

Projekt prezentuje dzialanie czterosuwowego silnika R4 w otwartym przekroju.
Animacja obejmuje tloki, korbowody, wal korbowy, zawory, dwa walki rozrzadu,
pompowtryskiwacze, lancuch rozrzadu oraz przeplyw mieszanki i spalin.

Program wykorzystuje nowoczesny OpenGL 3.3 Core, proceduralne modele 3D,
tekstury PNG, model oswietlenia Blinn-Phonga oraz dynamiczny shadow mapping.

## Glowne moduly

- `CameraController` obsluguje kamere orbitujaca i presety widoku.
- `EngineSimulator` zarzadza scena, RPM, oswietleniem i zespolami silnika.
- `Cylinder` oblicza mechanike pojedynczego cylindra w cyklu 720 stopni.
- `EngineMesh` generuje proceduralna geometrie i zarzadza VAO/VBO.
- `ShaderProgram` wczytuje i waliduje programy GLSL.

## Mechanika

Cylindry pracuja w kolejnosci zaplonu **1-3-4-2**. Tloki 1 i 4 poruszaja sie
wspolnie, a tloki 2 i 3 sa przesuniete mechanicznie o 180 stopni. Walki
rozrzadu obracaja sie z polowa predkosci walu korbowego.

Pelny cykl cylindra:

1. ssanie: 0-180 stopni,
2. sprezanie: 180-360 stopni,
3. praca: 360-540 stopni,
4. wydech: 540-720 stopni.

## Rendering

Kazda klatka sklada sie z dwoch przebiegow:

1. renderowania geometrii z pozycji lampy do mapy glebi 2048x2048,
2. renderowania sceny z kamery z uzyciem tekstur, Blinn-Phonga i mapy cienia.

Krawedzie cieni wygladza filtrowanie PCF 5x5. Transformacje modelu, widoku
i projekcji sa wykonywane za pomoca GLM.

## Budowanie

Windows MSYS2/MinGW:

```text
mingw32-make
./engine_simulator.exe
```

macOS:

```text
make
./engine_simulator
```

## Generowanie dokumentacji

Po zainstalowaniu Doxygen i Graphviz:

```text
mingw32-make docs
```

Wynik:

```text
docs/doxygen/html/index.html
```

Szczegolowy opis projektu przeznaczony do DokuWiki znajduje sie w pliku
`docs/sprawozdanie_dokuwiki_silnik_r4.txt`.

