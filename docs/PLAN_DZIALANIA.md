# Plan dzialania

Projekt prowadzimy malymi checkpointami. Po kazdym checkpointcie program ma sie kompilowac i uruchamiac.

## Checkpoint 1 - baza sceny

Cel: zamienic starter laboratoryjny w fundament projektu.

Zakres:

- uporzadkowana struktura repozytorium,
- jeden `Makefile` dla macOS i Windows/MSYS2,
- dzialajace okno OpenGL,
- kamera orbitujaca wokol sceny,
- pierwszy modul `EngineSimulator`,
- kilka widocznych obiektow 3D jako placeholder silnika,
- prosta animacja zalezna od RPM,
- sterowanie kamera i gazem.

Sterowanie:

Program startuje w pauzie, z minimalnym gazem.

- `A/D` - obrot kamery,
- `Q/E` - oddalenie/przyblizenie,
- `1/2/3/4` - presety kamery,
- `W/S` - gaz,
- `SPACE` - pauza/wznowienie,
- `R` - reset animacji, RPM i gazu oraz powrot do pauzy,
- `ESC` - wyjscie.

Status:

- build przez `mingw32-make` przechodzi,
- placeholder torusow zostal zastapiony proceduralnymi meshami `EngineMesh`,
- widac cztery cylindry, tloki, korbowody, wal i kolo zamachowe,
- to jeszcze nie jest finalna mechanika zaworow i korbowodow.

Dobry moment na push: po sprawdzeniu, ze okno startuje i widac animowane elementy.

## Checkpoint 2 - jeden cylinder

Cel: zastapic placeholder czytelnym jednym cylindrem.

Zakres:

- [x] wydzielic logike cylindra do klasy `Cylinder`,
- [x] dodac suw 0-720 stopni,
- [x] ruch tloka,
- [x] poprawnie ustawiony korbowod miedzy punktem tloka i czopem walu,
- [x] dwa zawory poruszane wedlug faz,
- [x] prosta obudowa cylindra w przekroju.

Status:

- `Cylinder` rysuje tuleje, komore, tlok, korbowod, czop i dwa zawory,
- zawor ssacy otwiera sie na ssaniu,
- zawor wydechowy otwiera sie na wydechu,
- komora cylindra ma kolor fazy pracy,
- blok silnika jest rysowany jako otwarty przekroj, zeby bylo widac tloki, korbowody i wal,
- cylindry maja polowiczny przekroj wzdluzny, zeby bylo widac ruch tloka w tulei,
- kazdy cylinder ma 4 zawory: 2 ssace i 2 wydechowe,
- widac sprezyny zaworowe, centralny wtrysk/swieca oraz proste dzwigienki,
- zawory podnosza sie przy aktywnej fazie i sa polaczone popychaczami z odpowiednimi walkami rozrzadu,
- dodana jest przekrojowa glowica z kanalami, prowadnicami i gniazdami zaworow,
- dodane sa uproszczone kolektory dolotowe i wydechowe,
- dodane sa pierscienie tlokowe, denka tlokow, sruby glowicy, lozyska walu i zeby kol rozrzadu.

Dobry moment na push: jeden cylinder ma tlok, wal/korbowe polaczenie i zawory reagujace na faze.

## Checkpoint 3 - silnik R4

Cel: rozbudowac jeden cylinder do czterech cylindrow.

Zakres:

- [x] 4 instancje cylindra,
- [x] przesuniecia faz,
- [x] wspolny wal,
- [x] kolo zamachowe,
- [x] blok silnika w przekroju,
- [x] widoczne czopy i przeciwwagi walu,
- [x] uproszczony uklad rozrzadu: dwa walki, krzywki, kola i animowany lancuch.

Dobry moment na push: widac cztery cylindry pracujace z roznymi fazami.

## Checkpoint 4 - interakcja

Cel: dopracowac sterowanie projektem.

Zakres:

- [x] plynne RPM,
- [x] pauza/wznowienie,
- [x] reset,
- [x] kamera i presety,
- [x] podstawowy HUD albo panel 3D.

Status:

- `W/S` zmienia gaz,
- RPM plynnie dochodzi do wartosci docelowej,
- `SPACE` przelacza pauze pojedynczym nacisnieciem,
- `R` resetuje animacje, RPM i gaz oraz wraca do pauzy,
- panel 3D pokazuje paski RPM/gazu i kontrolke pauzy.

## Checkpoint 5 - wymagania graficzne

Cel: spelnic wymagania oceniania.

Zakres:

- [x] minimum 2 tekstury,
- [x] minimum 2 zrodla swiatla,
- [x] widoczne cieniowanie,
- [x] poprawne normalne dla proceduralnych cylindrow i boxow,
- brak zakazanych funkcji starego OpenGL,
- brak koloru rozowego.

Status:

- dodany shader `v_engine.glsl` / `f_engine.glsl`,
- scena uzywa modelu Blinn-Phong,
- jest swiatlo kierunkowe i punktowe,
- metalowe elementy maja specular highlight,
- cylindry maja wiecej segmentow, a korbowody sa walcami zamiast boxow,
- generowane sa proceduralne tekstury: jasny metal, ciemny metal i ciemny material gumowy.

## Checkpoint 6 - finalizacja

Cel: przygotowac oddanie.

Zakres:

- README z uruchamianiem i sterowaniem,
- opis architektury,
- czyszczenie kodu,
- test na czysto,
- przygotowanie krotkiego demo.

## Polish wizualny

Status:

- dodane efekty faz pracy w cylindrach,
- ssanie ma niebieski kanal przeplywu,
- suw pracy ma pulsujacy blysk spalania,
- wydech ma ciemny przeplyw i proste chmurki spalin,
- sprezanie ma jasny znacznik w komorze.
