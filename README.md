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
├── docs/               # Dokumentation
├── include/            # Header-Dateien
│   ├── models/         # Datenmodelle (Property, Tenant, etc.)
│   ├── views/          # GUI- oder Konsolenansichten
│   ├── controllers/    # Steuerungsklassen
│   ├── ocr/            # OCR-Schnittstellen (z.B. IOcrEngine)
│   └── poppler/        # PDF-Renderer-Schnittstellen (z.B. IPdfRenderer)
├── src/                # Quellcode
│   ├── models/         # Implementierung der Datenmodelle
│   ├── views/          # Implementierung der Ansichten
│   ├── controllers/    # Implementierung der Steuerung
│   ├── ocr/            # Implementierung der OCR-Logik (z.B. TesseractOcrEngine)
│   └── poppler/        # Implementierung der PDF-Renderer (z.B. PopplerPdfRenderer)
├── CMakeLists.txt
└── README.md
```

## Build-Anleitung

1. **vcpkg klonen und initialisieren**  
   *(Nur nötig, wenn vcpkg noch nicht vorhanden ist. Überspringen, falls bereits vorhanden oder das Projekt inkl. vcpkg geklont wurde.)*
   ```bash
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   ./bootstrap-vcpkg.bat
   ```
2. **Projekt klonen**
   ```bash
   git clone https://github.com/dein-benutzername/fossredder.git
   cd fossredder
   ```
3. **Abhängigkeiten installieren**  
   *(Alle benötigten Bibliotheken werden automatisch anhand der `vcpkg.json` installiert.)*
   ```bash
   ../vcpkg/vcpkg install
   ```
   **Hinweis:** Stelle sicher, dass die Umgebungsvariable oder der CMake-Parameter `CMAKE_TOOLCHAIN_FILE` auf `../vcpkg/scripts/buildsystems/vcpkg.cmake` zeigt.  
   In Visual Studio 2022 wird dies bei Verwendung von CMake-Projekten und einer vorhandenen `CMakeSettings.json` in der Regel automatisch erkannt.

4. **Projekt mit CMake konfigurieren und bauen**  
   - **Kommandozeile:**
     ```bash
     cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
     cmake --build build
     ```
   - **Visual Studio 2022:**  
     Öffne das Projektordner direkt in Visual Studio. Die CMake-Integration erkennt die `CMakeSettings.json` und verwendet automatisch die vcpkg-Toolchain.

**Tipp:**  
Für weitere Details zur Einrichtung siehe die [offizielle vcpkg-Dokumentation](https://learn.microsoft.com/en-us/vcpkg/).

## Dokumentation

- [Pflichtenheft](docs/pflichtenheft.md)

## Lizenz

Dieses Projekt ist unter der MIT-Lizenz lizenziert. Weitere Details finden Sie in der [LICENSE](./LICENSE)-Datei.Dieses Projekt ist unter der MIT-Lizenz lizenziert. Weitere Details finden Sie in der [LICENSE](./LICENSE)-Datei.