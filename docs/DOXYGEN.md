# Dokumentacja Doxygen

Projekt zawiera gotowa konfiguracje `Doxyfile` oraz komentarze dokumentacyjne
w naglowkach klas. Dokumentacja HTML obejmuje architekture, klasy, metody,
pliki zrodlowe i grafy zaleznosci.

## Windows

Zainstaluj Doxygen i Graphviz. Najprostsze warianty:

- instalatory ze stron:
  - https://www.doxygen.nl/download.html
  - https://graphviz.org/download/
- albo Chocolatey uruchomione w PowerShellu jako administrator:

```powershell
choco install doxygen.install graphviz -y
```

Po instalacji otworz nowy terminal i sprawdz:

```powershell
doxygen --version
dot -V
```

W katalogu projektu uruchom:

```powershell
mingw32-make docs
```

## macOS

```sh
brew install doxygen graphviz
make docs
```

## Wynik

Strona glowna dokumentacji:

```text
docs/doxygen/html/index.html
```

Katalog `docs/doxygen/` jest generowany automatycznie i nie jest zapisywany
w repozytorium. Do paczki oddawanej prowadzacemu mozna dolaczyc wygenerowany
katalog `html`.

