# FOSSredder

**FOSSredder** ist eine spezialisierte Desktop-Anwendung zur lokalen Verwaltung von Ausgaben für private Vermieter mit mehreren Immobilien. Die Anwendung ermöglicht die Kategorisierung, Zuordnung und strukturierte Abrechnung sämtlicher Kosten. Zusätzlich unterstützt sie die Verarbeitung von Kontoauszügen im PDF-Format sowie den Import und Export von Excel- oder CSV-Dateien.

## Funktionen

- Verwaltung mehrerer Immobilien mit spezifischen Attributen
- Erstellung von Abrechnungen für einzelne oder alle Immobilien
- Kategorisierung von Ausgaben mit Markierung als „umlegbar“ oder „nicht umlegbar“
- Verarbeitung von PDF-Kontoauszügen mit heuristischer Extraktion von Beträgen und Kategorien
- KI-gestützte Extraktion und Kategorisierung von Buchungsinformationen (TinyLLaMA, optional)
- Visuelle Prüfung extrahierter Inhalte durch den Nutzer, inkl. Bestätigung, Korrektur oder Ignorieren
- Import/Export von CSV- oder Excel-Dateien
- Backup- und Wiederherstellungsfunktionen
- Analyse-Tools mit grafischer Auswertung
- Umschaltbarer Dark-/Lightmode
- Lokale Datenspeicherung ohne Cloud-Anbindung

## Technologie-Stack

- **Programmiersprache:** C++20
- **GUI:** Qt6
- **Buildsystem:** CMake
- **Abhängigkeitsverwaltung:** vcpkg
- **Datenspeicherung:** SQLite 
- **PDF-Verarbeitung:** Poppler, Tesseract, OpenCV
- **Plattform:** Windows 10+ (offline)

## Projektstruktur

```
fossredder/
├── app/                # Desktop-Entry-Point, i18n und Packaging
├── core/               # Domänenlogik, Analysen, Import/Export, Controller-Basis
├── debug/              # Debug- und Error-Reporting-Helfer
├── docs/               # Produkt- und Design-Dokumentation
├── persistence/        # SQLite-Backends und Persistenzadapter
├── services/           # Anbindungen an Poppler, OpenCV und Tesseract
├── ui/                 # Qt Widgets/QML, Controller und UI-State
├── CMakeLists.txt      # Root-Buildkonfiguration
└── vcpkg.json          # Abhängigkeiten
```

## Build-Anleitung

Die Abhängigkeiten werden in diesem Projekt über `vcpkg.json` im Manifest-Modus verwaltet. Das Repository enthält **keine** lokale `vcpkg`-Kopie und erwartet stattdessen eine **einmalig pro Entwicklerrechner** installierte `vcpkg`-Umgebung, die über `VCPKG_ROOT` angebunden wird.

### 1. `vcpkg` einmalig lokal installieren

Installiere `vcpkg` einmalig an einem **kurzen lokalen Pfad deiner Wahl**. Das konkrete Verzeichnis ist bewusst **deine lokale Entscheidung** und nicht im Repository fest verdrahtet.

Beispielhaft sieht das immer gleich aus:

```powershell
git clone https://github.com/microsoft/vcpkg.git <dein-vcpkg-pfad>
<dein-vcpkg-pfad>\bootstrap-vcpkg.bat
```

Falls `vcpkg` bereits lokal vorhanden ist, reicht es, die bestehende Installation weiterzuverwenden.

### 2. `VCPKG_ROOT` dauerhaft setzen

Setze anschließend die Benutzer-Umgebungsvariable `VCPKG_ROOT` **einmalig dauerhaft** auf dein lokales `vcpkg`-Verzeichnis:

```powershell
[Environment]::SetEnvironmentVariable('VCPKG_ROOT', '<dein-vcpkg-pfad>', 'User')
```

Danach gilt:

1. Das Projekt kennt deinen lokalen `vcpkg`-Pfad über `VCPKG_ROOT`.
2. Die Variable bleibt über Neustarts hinweg erhalten.
3. Wenn du **nichts weiter setzt**, legt das Projekt seine Manifest-Artefakte bei einem normalen lokalen `vcpkg` standardmäßig unter `VCPKG_ROOT\installed\fossredder` ab.
4. Das gilt sowohl für die Presets als auch für einen einfachen `cmake`-Aufruf, weil das Repository `CMAKE_TOOLCHAIN_FILE` und `VCPKG_INSTALLED_DIR` vor `project()` deterministisch aus `VCPKG_ROOT`, `VCPKG_INSTALLED_DIR` oder einem expliziten Toolchain-Pfad ableitet.

Das ist in diesem Repository der bevorzugte Standard, weil dadurch

1. `vcpkg` selbst,
2. die Paket-Installationen,
3. die zugehörigen Buildtrees

an einem gemeinsamen Ort bleiben, ohne den Projektordner zu vergrößern oder zusätzliche verstreute Ordner auf dem Rechner zu erzeugen.

### 3. Optional: `VCPKG_INSTALLED_DIR` selbst festlegen

Wenn du den Installations- und Buildtree-Pfad dennoch explizit steuern willst, kannst du zusätzlich **einmalig dauerhaft** `VCPKG_INSTALLED_DIR` setzen:

```powershell
[Environment]::SetEnvironmentVariable('VCPKG_INSTALLED_DIR', '<dein-vcpkg-pfad>\installed\fossredder', 'User')
```

Das ist optional. Ohne diese Variable verwendet das Projekt automatisch denselben Pfad als sauberen Standard unterhalb von `VCPKG_ROOT`.

Wenn `VCPKG_ROOT` in einer bestimmten Session nicht sauber durchgereicht wird, aber `VCPKG_INSTALLED_DIR` korrekt auf `<dein-vcpkg-pfad>\installed\fossredder` zeigt, kann das Repository daraus den lokalen `vcpkg`-Root ebenfalls deterministisch ableiten.

Wichtig: Eine explizit gesetzte Benutzer- oder Systemvariable `VCPKG_INSTALLED_DIR` übersteuert diese automatische Auswahl vollständig. Wenn Visual Studio bewusst mit einer anderen `vcpkg`-Toolchain konfiguriert wird, folgt das Projekt dieser Toolchain deterministisch; ein Repository-Fallback nach `.build\vcpkg_installed` existiert nicht mehr.

### 4. Repository klonen oder öffnen

```powershell
git clone https://github.com/Wiiii90/fossredder.git
cd fossredder
```

Falls das Repository bereits lokal vorhanden ist, genügt es, in den Projektordner zu wechseln.

### 5. Projekt konfigurieren

Diese README beschreibt bewusst den normalen **Kommandozeilen-Workflow** mit eigenem lokalem `vcpkg`. Es wird dabei **keine** Arbeit in der Visual-Studio-IDE vorausgesetzt.

Die CMake-Presets dieses Repositories verwenden den Generator `Visual Studio 18 2026`, werden aber ganz normal per `cmake` auf der Kommandozeile gestartet. Wenn `VCPKG_INSTALLED_DIR` nicht gesetzt ist, landen die installierten Pakete und die zugehörigen `vcpkg`-Buildtrees bei einem normalen lokalen `vcpkg` unter `VCPKG_ROOT\installed\fossredder`.

Das ist bewusst so gewählt:

1. kein `vcpkg_installed` im Repository,
2. keine Repo-abhängigen Langpfade für Qt-Builds,
3. kein zusätzlicher frei stehender Projektordner außerhalb von `vcpkg`.

Wichtig: Wenn du `VCPKG_ROOT`, `VCPKG_INSTALLED_DIR` oder `VCPKG_DEFAULT_BINARY_CACHE` neu gesetzt oder geändert hast, öffne **danach ein neues Terminal**, bevor du `cmake` startest.

**App konfigurieren:**

```powershell
cmake --preset app
```

**Tests konfigurieren:**

```powershell
cmake --preset tests
```

Die Presets sind nur die bequeme Kurzform. Der App-Configure entspricht inhaltlich ungefähr diesem expliziten CMake-Aufruf:

```powershell
cmake -S . -B .build/app -G "Visual Studio 18 2026" -A x64 -D VCPKG_TARGET_TRIPLET=x64-windows -D VCPKG_APPLOCAL_DEPS=ON
```

Beim ersten Konfigurieren installiert `vcpkg` die in `vcpkg.json` beschriebenen Abhängigkeiten automatisch.

### 6. Projekt bauen

**Release-App bauen:**

```powershell
cmake --build --preset release-app
```

**Debug-Tests bauen:**

```powershell
cmake --build --preset debug-tests
```

**Release-App starten:**

```powershell
.\.build\app\bin\Release\fossredder.exe
```

Alternativ kann für den Debug-Testlauf auch das vorhandene Skript verwendet werden:

```powershell
.\ci\build-debug.ps1 -RunTests
```

Das Skript prüft vor dem Konfigurieren, ob `VCPKG_ROOT` korrekt gesetzt ist.

### 7. Optional: Binary Cache aktivieren

Toolset-Wechsel oder Änderungen an der lokalen Buildumgebung können dazu führen, dass Pakete neu gebaut werden müssen. Das bedeutet normalerweise **nicht**, dass `vcpkg` selbst neu installiert werden muss. Damit solche Rebuilds nicht jedes Mal wieder viele Stunden kosten, empfiehlt sich ein lokaler Binary Cache.

Optional kann dafür einmalig eine weitere Benutzer-Umgebungsvariable gesetzt werden:

```powershell
[Environment]::SetEnvironmentVariable('VCPKG_DEFAULT_BINARY_CACHE', '<dein-vcpkg-pfad>\binarycache', 'User')
```

Wichtig: Das in `VCPKG_DEFAULT_BINARY_CACHE` referenzierte Verzeichnis muss tatsächlich existieren. Andernfalls bricht `vcpkg install` bereits beim Konfigurieren ab.

Damit bleiben auch Cache-Artefakte im selben `vcpkg`-Container und verteilen sich nicht an zusätzlichen Stellen auf dem Rechner.

## Dokumentation

- [Pflichtenheft](docs/pflichtenheft.md)

## Laufzeitdateien

- Das Release-Binary wird unter `out/build/bin/Release/` oder dem entsprechenden CMake-Binärverzeichnis abgelegt.
- Übersetzungen werden beim Build nach `bin/i18n/` kopiert.
- Die QML-Ressourcen stammen aus `ui/qml/`.

## Lizenz

Dieses Projekt ist unter der MIT-Lizenz lizenziert. Weitere Details finden Sie in der [LICENSE](./LICENSE)-Datei.
