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

Wähle für `vcpkg` einen **kurzen lokalen Pfad**, damit Windows-Pfadlängen kein Problem werden. Ein kurzer Pfad wie `P:\vcpkg` ist empfehlenswert, aber nicht vorgeschrieben.

```powershell
git clone https://github.com/microsoft/vcpkg.git P:\vcpkg
P:\vcpkg\bootstrap-vcpkg.bat
```

Falls `vcpkg` bereits lokal vorhanden ist, reicht es, die bestehende Installation weiterzuverwenden.

### 2. `VCPKG_ROOT` dauerhaft setzen

Setze anschließend die Benutzer-Umgebungsvariable `VCPKG_ROOT` **einmalig dauerhaft** auf dein lokales `vcpkg`-Verzeichnis:

```powershell
[Environment]::SetEnvironmentVariable('VCPKG_ROOT', 'P:\vcpkg', 'User')
```

Danach gilt:

1. Das Projekt kennt deinen lokalen `vcpkg`-Pfad über `VCPKG_ROOT`.
2. Die Variable bleibt über Neustarts hinweg erhalten.

### 3. Repository klonen oder öffnen

```powershell
git clone https://github.com/Wiiii90/fossredder.git
cd fossredder
```

Falls das Repository bereits lokal vorhanden ist, genügt es, in den Projektordner zu wechseln.

### 4. Projekt konfigurieren

Die CMake-Presets dieses Repositories verwenden automatisch `VCPKG_ROOT`. Die installierten Pakete landen dabei unter `.build/vcpkg_installed`, damit die Build-Artefakte kurzpfadig und projektbezogen bleiben, ohne `vcpkg` selbst ins Repository zu legen.

**App konfigurieren:**

```powershell
cmake --preset app
```

**Tests konfigurieren:**

```powershell
cmake --preset tests
```

Beim ersten Konfigurieren installiert `vcpkg` die in `vcpkg.json` beschriebenen Abhängigkeiten automatisch.

### 5. Projekt bauen

**Release-App bauen:**

```powershell
cmake --build --preset release-app
```

**Debug-Tests bauen:**

```powershell
cmake --build --preset debug-tests
```

Alternativ kann für den Debug-Testlauf auch das vorhandene Skript verwendet werden:

```powershell
.\ci\build-debug.ps1 -RunTests
```

Das Skript prüft vor dem Konfigurieren, ob `VCPKG_ROOT` korrekt gesetzt ist.

### 6. Visual Studio verwenden

Dieses Repository ist auf den Generator **Visual Studio 18 2026** ausgelegt. Öffne den Projektordner direkt in Visual Studio. Die CMake-Presets greifen dann auf dieselbe `VCPKG_ROOT`-Konfiguration zu wie die Kommandozeile.

Wenn Visual Studio `vcpkg` nicht findet, ist fast immer die Ursache, dass `VCPKG_ROOT` erst **nach** dem Start der IDE gesetzt wurde. In diesem Fall Visual Studio vollständig schließen und erneut öffnen.

## Dokumentation

- [Pflichtenheft](docs/pflichtenheft.md)

## Laufzeitdateien

- Das Release-Binary wird unter `out/build/bin/Release/` oder dem entsprechenden CMake-Binärverzeichnis abgelegt.
- Übersetzungen werden beim Build nach `bin/i18n/` kopiert.
- Die QML-Ressourcen stammen aus `ui/qml/`.

## Lizenz

Dieses Projekt ist unter der MIT-Lizenz lizenziert. Weitere Details finden Sie in der [LICENSE](./LICENSE)-Datei.
