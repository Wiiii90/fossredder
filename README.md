# FOSSredder

**FOSSredder** ist eine spezialisierte Desktop-Anwendung zur lokalen Verwaltung von Ausgaben für private Vermieter mit mehreren Immobilien. Die Anwendung ermöglicht die Kategorisierung, Zuordnung und strukturierte Abrechnung sämtlicher Kosten. Zusätzlich unterstützt sie die Verarbeitung von Kontoauszügen im PDF-Format sowie den Import und Export von Excel- oder CSV-Dateien.

## Funktionen

- Verwaltung mehrerer Immobilien mit spezifischen Attributen
- Erstellung von Abrechnungen für einzelne oder alle Immobilien
- Kategorisierung von Ausgaben mit Markierung als „umlegbar“ oder „nicht umlegbar“
- Verarbeitung von PDF-Kontoauszügen mit heuristischer Extraktion von Beträgen und Kategorien
- Visuelle Prüfung extrahierter Inhalte durch den Nutzer
- Import/Export von CSV- oder Excel-Dateien
- Backup- und Wiederherstellungsfunktionen
- Analyse-Tools mit grafischer Auswertung
- Lokale Datenspeicherung ohne Cloud-Anbindung

## Technologie-Stack

- **Programmiersprache:** C++17 oder höher
- **GUI:** Qt6
- **Buildsystem:** CMake
- **Abhängigkeitsverwaltung:** vcpkg
- **Datenspeicherung:** SQLite oder JSON
- **PDF-Verarbeitung**: Poppler, Tesseract, TinyLLaMA
- **Plattform:** Windows 10+ (offline)

## Projektstruktur


```
fossredder/
├── docs/         # Dokumentation
├── include/      # Header-Dateien
│   ├── models/   # Datenmodelle (Property, Tenant, etc.)
│   ├── views/    # GUI- oder Konsolenansichten
│   └── controllers/ # Steuerungsklassen
├── src/          # Quellcode
│   ├── models/   # Implementierung der Datenmodelle
│   ├── views/    # Implementierung der Ansichten
│   └── controllers/ # Implementierung der Steuerung
├── CMakeLists.txt
└── README.md
```

## Build-Anleitung

1. vcpkg klonen und initialisieren 
   ```bash
   git clone https://github.com/microsoft/vcpkg.git
   ./vcpkg/bootstrap-vcpkg.bat
   ```
2. Qt6 installieren (z. B. via Qt Online Installer oder vcpkg)
   ```bash
   ./vcpkg/vcpkg install qt6-base
   ```
3. Projekt klonen und CMake konfigurieren:
   ```bash
   git clone https://github.com/dein-benutzername/fossredder.git
   cd fossredder
   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
   ```
4. Build starten:
   ```bash
   cmake --build build
   ```

## Dokumentation

- [Pflichtenheft](docs/pflichtenheft.md)

## Lizenz

Dieses Projekt ist unter der MIT-Lizenz lizenziert. Weitere Details finden Sie in der [LICENSE](./LICENSE)-Datei.