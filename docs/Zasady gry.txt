Zasady gry
Zadania powinno się wykonywać w grupach dwuosobowych. Dopuszczalne są jednak solówki i grupy trzyosobowe. Grupy trzyosobowe będą oceniane ostrzej.
Tematy zostaną przedstawione na laboratoriach. Do 30 kwietnia 2026 starostowie poszczególnych grup laboratoryjnych (lub osoby wyznaczone) rozdzielą zadania w swoich grupach i prześlą odpowiednie listy do mnie.
Weryfikacja postępów prac odbędzie się w trzech terminach:
Początek Lab 8 - pokazanie wczytywania obiektów 3D 
Początek Lab 10 - pokazanie cieniowania wczytanych obiektów
Początek Lab 11 - pokazanie teksturowania wczytanych obiektów
W jednej grupie nie może być dwóch zespołów wykonujących ten sam temat.
Na koniec semestru zostaną wyznaczone terminy i sala oddawania projektów. Oddanie projektu polega na krótkiej demonstracji i odpowiedzeniu na kilka pytań.
Oddanie projektu po terminie może skończyć się tragicznie.
Jeżeli zostanie wykryty plagiat, to kończy się to automatycznie wpisaniem oceny niedostatecznej, bez możliwości poprawy.
Środowisko tworzenia projektu jest dowolne tak długo jak nie wymaga się od prowadzącego uruchamiania tego projektu samodzielnie.
W razie wykorzystywania jakiegoś egzotycznego kompilatora/środowiska nie przybiegać do prowadzącego z płaczem że coś nie działa.
Za nie egzotyczne środowiska uznawane są:
Linux+gcc we w miarę współczesnej wersji.
Visual Studio 2019 lub późniejsza.
Wymagania ogólne
Wykorzystanie OpenGL i GLM o ile prowadzący nie wyraził zgody na coś innego.
Scena zawiera kilka (więcej niż 4 i mniej niż nieskończoność) obiektów trójwymiarowych poruszających się niezależnie (choć to ostatnie może zależeć od projektu). W sytuacji gdy z jakiegoś powodu nie pasuje to do projektu należy dogadać się z prowadzącym.
Interakcja z użytkownikiem (np. poruszanie przedmiotem lub kamerą), bądź animacja automatyczna (zależy od projektu). Program nie powinien być statyczny.
Wszystkie obiekty są oteksturowane. Wykorzystano co najmniej dwie różne tekstury.
Scena jest oświetlona z przynajmniej dwóch źródeł światła. Cieniowanie powinno być widoczne, a nie tylko włączone w kodzie.
Nietrywialne modele 3D. Scena nie może składać się tylko z „gotowców” dostarczonych wraz z szkieletem programu do zajęć. Dopuszczalne jest pobranie gotowych modeli z sieci i wykorzystanie gotowej biblioteki do wczytania pliku z modelem. Kod rysujący dany model musi być jednak autorstwa studentów.
Co wolno
Wykorzystywać dowolne biblioteki zewnętrzne nie związane z grafiką komputerową.
Wykorzystywać modele ściągnięte z sieci.
Wykorzystywać dodatkowe programy do modelowania – dowolne.
Wykorzystywać istniejące biblioteki do wczytania modeli i plików graficznych. Każdą inną bibliotekę związaną w jakikolwiek sposób z grafiką komputerową należy konsultować z wykładowcą.
Czego nie wolno
Wykorzystywać gotowych procedur rysujących. Cały kod rysujący musi być waszego autorstwa i znany na wylot.
Używać koloru różowego
Wykorzystywać cudzych prac.
Indeks poleceń zakazanych:
glBegin, glEnd – zamiast tego nalezy używać glDrawArrays/glDrawElements
glVertex, glNormal, glTexCoord – zamiast tego nalezy używać glDrawArrays/glDrawElements
glRotate, glTranslate, glScale – zamiast tego nalezy używać procedur rotate/translate/scale z biblioteki GLM
gluLookat – zamiast tego nalezy używać procedury lookAt z biblioteki GLM
glFrustum, gluPerspective, glOrtho – zamiast tego należy uzywać procedur frustum/perspective/ortho z bibioteki GLM
glCreateList, glDeleteList, glCallList – zamiast tego nalezy używać glDrawArrays/glDrawElements
glPushMatrix, glPopMatrix – zamiast tego nalezy używać dowolnych struktur danych przechowujących obiekty mat4 z biblioteki GLM
glVertexPointer, glNormalPointer, glTexCoordPointer, glColorPointer – zamiast tego używać glVertexAttribPointer
glEnableClientState, glDisableClientState – zamiast tego używać glEnableVertexAttribArray/glDisableVertexAttribArray
Ogólne reguły oceniania projektów
Nie da się wszystkiego przewidzieć i dlatego każdy przypadek będzie rozważany indywidualnie, ale ogólne reguły są następujące. Początkowa ocena 2.0…., a potem:
Wykorzystanie nietrywialnych modeli (najlepiej odczytanych z pliku): +0.5 do oceny
Poprawne zarządzanie obiektami na scenie (poprawne poruszanie, animacja i dobre wykorzystanie kamery): +1 do oceny
Poprawne teksturowanie obiektów oraz poprawne wykorzystanie mechanizmów OpenGL do rysowania: +0.5 do oceny
Poprawne ocieniowanie obiektów: +1 do oceny
Uwaga! Jeżeli się okaże że nie wiecie jak działa jakiś fragment waszego programu - but.
W przypadku grupy trzyosobowej prowadzący będzie upierdliwy jak w sierżant wojsku. Poza tym reguły oceniania takie same.