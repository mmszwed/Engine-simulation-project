# Przygotowanie do obrony projektu

Ten dokument opisuje, jak dziala projekt, gdzie znajduja sie najwazniejsze
fragmenty kodu oraz jak odpowiedziec na typowe pytania prowadzacego.

## Wypowiedz na poczatek

Mozesz zaczac mniej wiecej tak:

> Projekt jest interaktywna wizualizacja czterosuwowego silnika R4 wykonana
> w C++ i nowoczesnym OpenGL 3.3 Core. Cala geometria silnika jest generowana
> proceduralnie i przechowywana w VAO oraz VBO. Animacja bazuje na wspolnym
> kacie walu w zakresie 0-720 stopni, z ktorego wyliczane sa pozycje tlokow,
> korbowodow, zaworow, walkow rozrzadu, wtrysku i lancucha. Cylindry pracuja
> w kolejnosci zaplonu 1-3-4-2. Scena wykorzystuje tekstury, model
> Blinn-Phonga, dwie lampy warsztatowe oraz shadow mapping. Uzytkownik moze
> sterowac kamera, RPM, pauza, resetem i oswietleniem.

## Przeplyw programu

Najwazniejszy przeplyw jednej klatki:

1. `main()` tworzy okno i kontekst OpenGL.
2. `CameraController::update()` odczytuje klawisze kamery.
3. `EngineSimulator::update()` aktualizuje gaz, RPM i kat walu.
4. `drawScene()` renderuje mape cienia z pozycji lampy.
5. `drawScene()` renderuje normalna scene z pozycji kamery.
6. GLFW zamienia przedni i tylny bufor obrazu.

## main_file.cpp

### `main()`

Glowny punkt wejscia programu.

Wykonuje kolejno:

- inicjalizacje GLFW,
- ustawienie OpenGL 3.3 Core Profile,
- utworzenie okna 1000x700,
- inicjalizacje GLEW,
- wywolanie `initOpenGLProgram()`,
- petle programu,
- zwolnienie zasobow.

W petli obliczany jest `deltaTime`:

```cpp
currentTime = glfwGetTime();
deltaTime = currentTime - previousTime;
```

`deltaTime` powoduje, ze predkosc kamery i animacji nie zalezy bezposrednio
od liczby klatek na sekunde.

### `initOpenGLProgram()`

Przygotowuje zasoby wymagane przed renderowaniem:

- kompiluje programy shaderow,
- wlacza test glebi przez `glEnable(GL_DEPTH_TEST)`,
- tworzy glowne VAO,
- ustawia viewport,
- tworzy framebuffer mapy cienia,
- tworzy teksture glebi 2048x2048.

Framebuffer mapy cienia ma tylko bufor glebi. Nie potrzebuje bufora koloru,
dlatego ustawiane sa:

```cpp
glDrawBuffer(GL_NONE);
glReadBuffer(GL_NONE);
```

### `drawScene()`

Najwazniejsza funkcja renderujaca.

Pierwszy przebieg:

- ustawia kamere w pozycji lampy,
- podpina `shadowFramebuffer`,
- czysci bufor glebi,
- wywoluje `engine.drawShadow()`.

Drugi przebieg:

- wraca do domyslnego framebuffera okna,
- ustawia viewport rozmiaru okna,
- oblicza macierz kamery i perspektywy,
- podpina teksture mapy cienia,
- wywoluje `engine.draw()`.

To jest klasyczny shadow mapping.

### `freeOpenGLProgram()`

Usuwa:

- meshe i tekstury silnika,
- teksture mapy cienia,
- framebuffer cienia,
- programy shaderow,
- glowne VAO.

Zasoby GPU trzeba zwalniac jawnie, poniewaz nie sa zwykla pamiecia C++.

## CameraController

### Stan kamery

Kamera przechowuje:

- `yaw` - obrot poziomy,
- `pitch` - obrot pionowy,
- `distance` - odleglosc od silnika.

### `update()`

Odczytuje klawiature:

- `A/D` zmienia `yaw`,
- `W/S` zmienia `pitch`,
- `Q/E` zmienia odleglosc,
- `1-4` ustawia preset.

`std::clamp` ogranicza odleglosc i pionowy obrot kamery. Zapobiega to
wejsciu kamery do modelu oraz odwroceniu widoku do gory nogami.

### `getPosition()`

Przelicza wspolrzedne sferyczne na kartezjanskie:

```text
horizontal = distance * cos(pitch)
x = horizontal * sin(yaw)
y = distance * sin(pitch)
z = horizontal * cos(yaw)
```

### `getViewMatrix()`

Tworzy macierz widoku:

```cpp
glm::lookAt(position, target, up);
```

Kamera znajduje sie w `position`, patrzy na srodek silnika, a dodatni kierunek
osi Y jest kierunkiem do gory.

## EngineSimulator

Jest glownym koordynatorem projektu.

Przechowuje:

- wspolny `crankAngle`,
- aktualne i docelowe RPM,
- gaz,
- stan pauzy i lamp,
- cztery cylindry,
- proceduralne meshe,
- tekstury,
- elementy silnika i otoczenia.

### Konstruktor

Tworzy cztery cylindry:

```cpp
cylinders.emplace_back(0, -2.1f, 0.0f, 360.0f);
cylinders.emplace_back(1, -0.7f, 180.0f, 540.0f);
cylinders.emplace_back(2, 0.7f, 180.0f, 180.0f);
cylinders.emplace_back(3, 2.1f, 0.0f, 0.0f);
```

`crankOffset` odpowiada za mechaniczne polozenie czopa walu. Tloki 1 i 4
poruszaja sie razem, a tloki 2 i 3 sa przesuniete o 180 stopni.

`cycleOffset` odpowiada za suw danego cylindra. Te przesuniecia daja
kolejnosc zaplonu 1-3-4-2.

### `update()`

Obsluguje:

- strzalki gora/dol - gaz,
- `SPACE` - pauza,
- `R` - reset,
- `L` - lampy.

Flagi `spaceWasPressed`, `resetWasPressed` i `lampWasPressed` realizuja
wykrycie pojedynczego nacisniecia. Bez nich stan zmienialby sie wiele razy
podczas przytrzymania jednego klawisza.

Gaz jest ograniczony do zakresu 0-1:

```cpp
throttle = std::clamp(throttle, 0.0f, 1.0f);
```

Docelowe RPM:

```cpp
targetRpm = minRpm + throttle * (maxRpm - minRpm);
```

Plynne dochodzenie do obrotow:

```cpp
rpm += (targetRpm - rpm) * 2.2f * deltaTime;
```

Kat walu:

```cpp
degreesPerSecond = (rpm / 60.0f) * 360.0f * animationScale;
crankAngle = fmod(crankAngle + degreesPerSecond * deltaTime, 720.0f);
```

Zakres wynosi 720 stopni, poniewaz pelny cykl czterosuwowy wymaga dwoch
obrotow walu.

### `draw()`

Rysuje normalna scene:

- garaz,
- stol,
- lampy,
- kanister,
- obudowe silnika,
- cztery cylindry,
- wal korbowy,
- rozrzad,
- kolektory,
- panel.

Funkcja nie oblicza ruchu od nowa. Przekazuje aktualny `crankAngle` do
elementow animowanych.

### `drawShadow()`

Rysuje tylko elementy, ktore maja rzucac cien. Uzywa macierzy widoku i
projekcji lampy oraz prostego shadera glebi.

Nie rysuje calego garazu, poniewaz glownym celem mapy jest cien silnika,
kanistra i panelu na otoczeniu.

### `initMeshes()`

Tworzy proceduralne meshe tylko raz. Flaga `meshesReady` zapobiega ponownemu
tworzeniu VAO i VBO w kazdej klatce.

Tworzone sa:

- box,
- cylinder,
- krzywka,
- kola zebate 12 i 24 zeby,
- polcylinder,
- otwarte kolektory,
- plyta zaworowa,
- pierscien.

### `initTextures()`

Tworzy trzy male tekstury proceduralne:

- jasny metal,
- ciemny metal,
- guma.

Laduje tez cztery tekstury PNG:

- drewno,
- sciane garazu,
- beton,
- odlewane aluminium.

### `createProceduralTexture()`

Generuje tablice pikseli RGB w pamieci. W zaleznosci od wariantu zmienia
jasnosc i wzor materialu, a pozniej przesyla wynik do GPU.

### `loadPngTexture()`

Uzywa LodePNG do odczytania pliku PNG jako tablicy RGBA. Nastepnie:

- tworzy uchwyt tekstury,
- wywoluje `glTexImage2D`,
- generuje mipmapy,
- ustawia powtarzanie i filtrowanie.

### `chooseTexture()`

Wybiera proceduralna teksture na podstawie jasnosci koloru:

- bardzo ciemny kolor -> guma,
- srednio ciemny -> ciemny metal,
- jasny -> jasny metal.

Jest to uproszczony system materialow.

### Funkcje otoczenia

- `drawGarageShell()` - podloga, sciany, slupy i belki garazu.
- `drawWorkshopStand()` - blat, nogi i wsporniki stolu.
- `drawWallLamp()` - lampa zamocowana na scianie.
- `drawWorkLamp()` - stojaca lampa warsztatowa.
- `drawFuelCanister()` - kanister, uchwyt, korek i przewod paliwowy.
- `drawStatusPanel()` - paski RPM, gazu i kontrolka pauzy.

Wszystkie te obiekty powstaja z tych samych proceduralnych meshy. Roznia sie
macierzami modelu i materialami.

### `drawEngineBlockCutaway()`

Rysuje obudowe w przekroju:

- tylna sciane skrzyni korbowej,
- kolnierz,
- miske olejowa,
- scianki boczne,
- wewnetrzne przegrody.

Przednia czesc jest celowo otwarta, aby bylo widac mechanike.

### `drawCrankshaftAssembly()`

Rysuje wspolny wal korbowy.

Dla kazdego cylindra wylicza czop:

```text
y = mainY + r * sin(theta)
z = r * cos(theta)
```

Tworzy:

- czopy glowne,
- czopy korbowe,
- policzki walu,
- ramiona,
- przeciwwagi,
- lozyska.

Przeciwwagi znajduja sie po przeciwnej stronie czopa, aby wizualnie
rownowazyc uklad.

### `drawValveTrain()`

Rysuje dwa walki rozrzadu, krzywki, lozyska, kola rozrzadu i mechanizm
pompowtryskiwaczy.

Walek rozrzadu obraca sie dwa razy wolniej:

```cpp
camAngle = crankAngle * 0.5f;
```

Powod: w silniku czterosuwowym wal wykonuje dwa obroty na jeden cykl, a walek
rozrzadu jeden.

Krzywki dla kazdego cylindra otrzymuja przesuniecie wynikajace z jego fazy.
Oddzielne rzedy obsluguja zawory dolotowe i wydechowe.

Wtrysk jest aktywny w krotkim zakresie przed suwem pracy. Funkcja sinus
tworzy plynne wcisniecie i powrot popychacza.

### `drawTimingChain()`

Buduje zamknieta trase lancucha z:

- odcinkow prostych,
- lukow wokol trzech kol.

Nastepnie:

1. oblicza dlugosc calej trasy,
2. dobiera liczbe ogniw,
3. dla kazdego ogniwa wyznacza odleglosc na trasie,
4. znajduje odpowiedni segment,
5. interpoluje pozycje przez `glm::mix`,
6. ustawia ogniwo zgodnie ze styczna trasy.

Przesuniecie `travel` zalezy od kata walu. Znak minus ustala poprawny kierunek
ruchu lancucha.

### `drawManifolds()`

Rysuje:

- magistrale dolotowa,
- magistrale wydechowa,
- galezie prowadzace do zaworow,
- magistrale paliwowa,
- znaczniki przeplywu powietrza, spalin i paliwa.

Krzywe rur sa przyblizane wieloma krotkimi segmentami. Punkty posrednie sa
liczone na krzywej Beziera.

### `drawMesh()`

Wygodny wrapper. Wybiera teksture wedlug koloru i wywoluje
`drawTexturedMesh()`.

### `drawTexturedMesh()`

Przed rysowaniem pojedynczego obiektu:

- aktywuje shader,
- wysyla macierze P, V i M,
- wysyla kolor,
- przelicza swiatla do przestrzeni widoku,
- wysyla parametry lamp,
- podpina teksture,
- wywoluje `mesh.draw()`.

### `drawUnlitMesh()`

Rysuje element bez obliczania oswietlenia. Jest uzywane miedzy innymi dla
swiecacych powierzchni lamp.

## Cylinder

Kazdy obiekt `Cylinder` zna:

- numer cylindra,
- polozenie X,
- przesuniecie mechaniczne walu,
- przesuniecie cyklu 720 stopni.

### `phase()`

Dodaje `cycleOffset` do wspolnego kata walu i zawija wynik do zakresu 0-720.

```cpp
p = fmod(crankAngle + cycleOffset, 720.0f);
```

### `getStroke()`

Przypisuje faze do suwu:

- 0-180: ssanie,
- 180-360: sprezanie,
- 360-540: praca,
- 540-720: wydech.

### `pistonPinHeight()`

Oblicza polozenie sworznia tloka z rzeczywistej geometrii mechanizmu
korbowego:

```text
crankY = mainY + r * sin(theta)
crankZ = r * cos(theta)
pistonY = crankY + sqrt(l^2 - crankZ^2)
```

`r` to promien wykorbienia, a `l` to dlugosc korbowodu.

To jest dokladniejsze niz zwykla sinusoida, poniewaz korbowod ma stala
dlugosc i przechyla sie podczas obrotu walu.

### `valveOffset()`

Wyznacza otwarcie zaworu w aktywnym suwie. Uzywa funkcji sinus podniesionej
do kwadratu:

```text
lift = maxLift * sin(pi * t)^2
```

Dzieki temu zawor:

- zaczyna od zerowej predkosci,
- plynnie sie otwiera,
- osiaga maksimum w srodku fazy,
- plynnie wraca do gniazda.

### `alignCylinderBetween()`

Ustawia cylinder pomiedzy dwoma punktami:

1. oblicza srodek odcinka,
2. oblicza jego kierunek i dlugosc,
3. wyznacza os obrotu iloczynem wektorowym,
4. obraca bazowy cylinder,
5. skaluje go do wymaganej dlugosci.

Ta funkcja jest uzywana dla korbowodow, trzpieni, prowadnic i sprezyn.

### `draw()`

Glowna funkcja pojedynczego cylindra.

Kolejnosc:

- oblicza faze i suw,
- wylicza czop walu i polozenie tloka,
- rysuje tuleje w przekroju,
- rysuje komore spalania,
- rysuje tlok i korbowod,
- rysuje plyte zaworowa,
- wylicza bezpieczne otwarcie zaworow,
- rysuje cztery zawory,
- rysuje pompowtryskiwacz,
- rysuje efekty aktualnego suwu.

`safeLift` ogranicza otwarcie zaworu, aby nie wszedl w poruszajacy sie tlok.

### `drawValvePair()`

Jedno wywolanie rysuje dwa zawory po tej samej stronie cylindra.

Kazdy zawor zawiera:

- gniazdo,
- prowadnice,
- trzpien,
- talerzyk,
- elementy mocujace,
- popychacz,
- sprezyne.

Dolot i wydech maja przeciwne pochylenie osi.

### `drawInjector()`

Rysuje centralny pompowtryskiwacz:

- dysze,
- iglice,
- korpus pompy,
- sprezyny,
- zlacze paliwowe,
- popychacz.

Widoczny impuls paliwa pojawia sie tylko w odpowiednim zakresie fazy.

### `drawStrokeEffects()`

Tworzy efekty czterech suwow:

- ssanie: niebieskie czasteczki schodza do cylindra,
- sprezanie: chmura zmniejsza promien,
- praca: krotki zolty rozblysk i rozszerzanie gazu,
- wydech: ciemniejsze czasteczki ida do zaworow wydechowych.

### `drawSpring()`

Sprezyna nie jest gotowym modelem. Jest tworzona jako helisa:

```text
x = cos(angle) * radius
z = sin(angle) * radius
y = interpolacja(bottom, top, t)
```

Kolejne punkty helisy sa laczone cienkimi cylindrami.

### Puste funkcje

`drawHeadFasteners()` i `drawCutawayEdges()` sa obecnie pustymi punktami
rozszerzenia. Nie wplywaja na scene. Na obronie najlepiej powiedziec wprost,
ze zostaly po iteracyjnym rozwoju modelu i moglyby zostac usuniete w dalszym
cleanupie.

## EngineMesh

### Format wierzcholka

Kazdy wierzcholek ma 10 liczb:

- 4 skladowe pozycji,
- 4 skladowe normalnej,
- 2 wspolrzedne UV.

### `upload()`

Tworzy VAO i VBO, przesyla dane przez `glBufferData` i ustawia trzy atrybuty:

- location 0: pozycja,
- location 1: normalna,
- location 2: UV.

`glVertexAttribPointer` opisuje GPU, gdzie w rekordzie wierzcholka znajduje
sie kazdy atrybut.

### `draw()`

Binduje VAO i wykonuje:

```cpp
glDrawArrays(GL_TRIANGLES, 0, vertexCount);
```

Nie sa uzywane zakazane `glBegin`, `glEnd` ani `glVertex`.

### Generatory

- `createBox()` - szesc scian z poprawnymi normalnymi.
- `createCylinder()` - bok oraz dwa zamkniecia cylindra.
- `createCamLobe()` - promien zalezy od kata, tworzac nos krzywki.
- `createSprocket()` - wypelnione kolo i nieruchome zeby.
- `createHalfCylinder()` - polowa tulei do przekroju.
- `createPortedHalfCylinder()` - otwarta rura z miejscami na odgalezienia.
- `createValvePlate()` - plyta z czterema otworami zaworow i otworem centralnym.
- `createRing()` - pierscien uzywany jako gniazdo lub kolnierz.

## ShaderProgram

### `readFile()`

Otwiera plik GLSL, sprawdza jego rozmiar, czyta cala zawartosc i dodaje znak
konca tekstu.

Na Visual Studio uzywa `fopen_s`, a na GCC/MinGW/macOS `fopen`.

### `loadShader()`

Tworzy shader, przekazuje kod z pliku, kompiluje go i sprawdza
`GL_COMPILE_STATUS`. W przypadku bledu wypisuje log sterownika OpenGL.

### Konstruktor

Laduje vertex shader, opcjonalny geometry shader i fragment shader.
Nastepnie laczy je w jeden program i sprawdza `GL_LINK_STATUS`.

### `use()`, `u()`, `a()`

- `use()` aktywuje program przez `glUseProgram`.
- `u()` pobiera lokalizacje uniformu.
- `a()` pobiera lokalizacje atrybutu.

## Vertex shader

Plik `shaders/v_engine.glsl`.

Wykonuje:

```glsl
viewPos = V * M * vertex;
gl_Position = P * viewPos;
```

Macierze:

- `M` - model, polozenie pojedynczego obiektu,
- `V` - view, kamera,
- `P` - projection, perspektywa.

Normalna jest transformowana macierza:

```glsl
transpose(inverse(mat3(V * M)))
```

Jest to potrzebne, poniewaz zwykle przeksztalcenie modelu moze zawierac
nierownomierne skalowanie, ktore znieksztalciloby normalne.

Shader wylicza tez polozenie fragmentu w przestrzeni lampy dla mapy cienia.

## Fragment shader

Plik `shaders/f_engine.glsl`.

### Teksturowanie

Probkuje `texture0` przez wspolrzedne UV. Kolor tekstury jest mnozony przez
kolor materialu.

### Blinn-Phong

Kolor koncowy sklada sie z:

- ambient,
- diffuse,
- specular.

Diffuse:

```text
max(dot(normal, lightDirection), 0)
```

Specular uzywa wektora polowicznego pomiedzy swiatlem a obserwatorem:

```text
halfway = normalize(light + view)
pow(max(dot(normal, halfway), 0), shininess)
```

### Reflektory

Iloczyn skalarny kierunku reflektora i kierunku do fragmentu okresla, czy
fragment znajduje sie w stozku swiatla. `smoothstep` tworzy miekka krawedz.

### Tlumienie

Swiatlo punktowe slabnie z odlegloscia:

```text
1 / (1 + linear * distance + quadratic * distance^2)
```

### `shadowAmount()`

1. Dzieli wspolrzedne przez `w`.
2. Przeksztalca zakres z -1..1 na 0..1.
3. Pobiera najblizsza glebokosc z mapy.
4. Porownuje ja z glebokoscia aktualnego fragmentu.
5. Wykonuje 25 probek PCF w siatce 5x5.

`bias` przesuwa porownanie i ogranicza samoocienianie powierzchni.

## Shader cienia

`v_shadow.glsl` wykonuje tylko:

```glsl
gl_Position = P * V * M * vertex;
```

`f_shadow.glsl` jest pusty, poniewaz framebuffer zapisuje tylko glebokosc.

## VAO, VBO, tekstury i framebuffer

### VAO

Vertex Array Object przechowuje konfiguracje atrybutow wierzcholkow.

### VBO

Vertex Buffer Object przechowuje dane wierzcholkow w pamieci GPU.

### Tekstura

Dwuwymiarowa tablica danych. W projekcie tekstury przechowuja kolory
materialow oraz mape glebi cienia.

### FBO

Framebuffer Object pozwala renderowac poza ekranem. W projekcie FBO sluzy
do zapisania sceny widzianej przez lampe do tekstury glebi.

## Dlaczego nie ma EBO?

Projekt uzywa `glDrawArrays`, wiec wierzcholki trojkatow sa zapisane
bezposrednio w VBO. EBO bylby potrzebny przy `glDrawElements`, aby wiele
trojkatow moglo wspoldzielic te same indeksowane wierzcholki.

Oba podejscia sa nowoczesnym OpenGL i sa dozwolone przez wymagania.

## Uproszczenia projektu

Warto umiec je nazwac:

- nie jest to symulacja CFD przeplywu gazow,
- nie ma rzeczywistego spalania ani termodynamiki,
- czasteczki tylko wizualizuja aktualny suw,
- kolektory sa przyblizone proceduralnymi segmentami,
- materialy uzywaja Blinn-Phonga, a nie pelnego PBR,
- jedna mapa cienia jest liczona dla stojacej lampy,
- panel pokazuje wartosci paskami bez tekstowego wyswietlacza RPM.

To sa swiadome uproszczenia wizualizacji edukacyjnej.

## Najczestsze pytania i odpowiedzi

### Dlaczego cykl ma 720, a nie 360 stopni?

Wal korbowy musi wykonac dwa obroty, aby cylinder przeszedl przez cztery suwy.

### Dlaczego walek rozrzadu obraca sie dwa razy wolniej?

Zawory musza wykonac jeden cykl otwarcia podczas dwoch obrotow walu.

### Skad kolejnosc 1-3-4-2?

Kazdy cylinder ma odpowiedni `cycleOffset`. Przesuniecia faz sprawiaja, ze
suw pracy zaczyna sie kolejno w cylindrach 1, 3, 4 i 2.

### Jak zachowujecie stala dlugosc korbowodu?

Pozycja tloka jest liczona ze wzoru zawierajacego pierwiastek
`sqrt(l^2 - z^2)`, gdzie `l` jest stala dlugoscia korbowodu.

### Jak dziala otwieranie zaworu?

Funkcja sinus do kwadratu tworzy plynny profil podnoszenia. Zawor zaczyna
i konczy ruch z zerowym podniesieniem.

### Po co sa normalne?

Normalna opisuje kierunek powierzchni. Shader porownuje ja z kierunkiem
swiatla, aby policzyc jasnosc i odbicia.

### Co robi macierz normalna?

Poprawnie transformuje normalne po skalowaniu i obrocie modelu. Jest to
`transpose(inverse(mat3(V * M)))`.

### Jak dziala shadow mapping?

Najpierw scena jest renderowana z pozycji lampy do mapy glebi. Potem dla
kazdego fragmentu sprawdzamy, czy z punktu widzenia lampy znajduje sie za
inna powierzchnia.

### Co to jest PCF?

Percentage-Closer Filtering. Zamiast jednej probki mapy cienia bierzemy wiele
probek wokol punktu i usredniamy wynik, uzyskujac lagodniejsza krawedz.

### Po co jest bias?

Bez biasu fragment moze uznac wlasna powierzchnie za przeszkode przez bledy
precyzji i powstaje wzor kropek na oswietlonej powierzchni.

### Dlaczego swiatla sa przeliczane macierza widoku?

Pozycje fragmentow i normalne w shaderze sa w przestrzeni widoku. Wszystkie
wektory uzyte w jednym obliczeniu musza byc w tej samej przestrzeni.

### Co zapewnia test glebi?

GPU zachowuje fragment o najmniejszej glebi. Dzieki temu obiekty znajdujace
sie z tylu nie sa rysowane na obiektach z przodu.

### Dlaczego uzywacie GLM?

GLM dostarcza wektory i macierze zgodne ze stylem GLSL. Wymagania zabraniaja
starych `glTranslate`, `glRotate` i `glScale`, wiec transformacje wykonujemy
przez `glm::translate`, `glm::rotate` i `glm::scale`.

### Jakie polecenie faktycznie rysuje obiekty?

`glDrawArrays(GL_TRIANGLES, 0, vertexCount)` w `EngineMesh::draw()`.

### Czy korzystacie z gotowych modeli?

Nie. Geometria silnika i otoczenia jest generowana w `EngineMeshes.cpp`
i skladana macierzami modelu.

### Skad sa tekstury?

Pliki PNG sa zasobami projektu, a ich dekodowanie wykonuje LodePNG. Trzy
dodatkowe tekstury materialow sa generowane proceduralnie w kodzie.

### Jak zwalniacie zasoby?

Meshe usuwaja VAO i VBO, tekstury sa usuwane przez `glDeleteTextures`,
framebuffer przez `glDeleteFramebuffers`, a shadery i programy przez
`glDeleteShader` oraz `glDeleteProgram`.

## Co pokazac podczas obrony

1. Uruchom projekt, ktory startuje w pauzie.
2. Presetem 1 pokaz caly silnik.
3. Nacisnij `SPACE` i pokaz wolne obroty.
4. Presetem 2 lub 3 pokaz tloki i wal.
5. Wyjasnij pary tlokow 1+4 oraz 2+3.
6. Pokaz zawory i dwa walki rozrzadu.
7. Wlacz `L` i pokaz roznice w oswietleniu oraz cienie.
8. Zwieksz RPM strzalka w gore.
9. Pokaz panel i efekty suwow.
10. Nacisnij `R`, aby pokazac reset.

## Minimum do nauczenia

Jesli masz malo czasu, zapamietaj:

- przeplyw `main -> update -> shadow pass -> normal pass`,
- macierze P, V i M,
- VAO, VBO i `glDrawArrays`,
- wzor ruchu tloka i stala dlugosc korbowodu,
- cykl 720 stopni i kolejnosc 1-3-4-2,
- walek rozrzadu obraca sie z polowa predkosci,
- Blinn-Phong: ambient, diffuse, specular,
- shadow mapping: mapa glebi, porownanie, bias, PCF,
- tekstury sa ladowane przez LodePNG,
- wszystkie modele sa proceduralne,
- projekt nie uzywa zakazanych funkcji starego OpenGL.

