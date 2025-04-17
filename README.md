# FOSSredder

**FOSSredder** ist eine spezialisierte Desktop-Anwendung zur lokalen Verwaltung von Ausgaben für einen privaten Vermieter mit mehreren Immobilien. Die Anwendung ermöglicht die Kategorisierung, Zuordnung und strukturierte Abrechnung sämtlicher Kosten. Zusätzlich unterstützt sie die Verarbeitung von Kontoauszügen im PDF-Format sowie den Import und Export von Excel- oder CSV-Dateien.

## Funktionen

- Verwaltung mehrerer Immobilien mit spezifischen Attributen (z. B. Name, Adresse).
- Erstellung von Abrechnungen (monatlich oder jährlich) für einzelne Immobilien oder alle Immobilien zusammen.
- Kategorisierung von Ausgaben mit der Möglichkeit, Kategorien als „umlegbar“ oder „nicht umlegbar“ zu markieren.
- Verarbeitung von PDF-Dateien (z. B. Kontoauszüge) mit automatischer Extraktion relevanter Daten (z. B. Betrag, Kategorie, Datum).
- Import und Export von Excel- oder CSV-Dateien.
- Backup- und Wiederherstellungsfunktionen für alle Systemdaten.
- Moderne GUI mit Analyse-Tools zur Visualisierung von Daten (z. B. Diagramme, Plots).
- Lokale Datenspeicherung (kein Cloud-Zwang).

## Technologie-Stack

- Programmiersprache: C++17 oder höher
- GUI: Qt6
- Buildsystem: CMake
- Datenspeicherung: SQLite oder JSON (konfigurierbar)
- Plattform: Windows 10+ (lokal, offline)

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

1. Qt6 installieren (z. B. via Qt Online Installer oder vcpkg)
2. Projekt klonen:
   ```bash
   git clone https://github.com/dein-benutzername/fossredder.git
   cd fossredder
   ```
3. Mit CMake konfigurieren:
   ```bash
   cmake -B build -S . -G "Ninja" -DCMAKE_BUILD_TYPE=Release
   ```
4. Build starten:
   ```bash
   cmake --build build
   ```

## Dokumentation

- [Pflichtenheft](docs/pflichtenheft.md)

## Lizenz

Dieses Projekt ist unter der MIT-Lizenz lizenziert. Weitere Details finden Sie in der [LICENSE](./LICENSE)-Datei.