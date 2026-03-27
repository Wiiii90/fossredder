# FOSSredder

[![Quality](https://github.com/Wiiii90/fossredder/actions/workflows/quality.yml/badge.svg)](https://github.com/Wiiii90/fossredder/actions/workflows/quality.yml)
[![codecov](https://codecov.io/gh/Wiiii90/fossredder/graph/badge.svg)](https://codecov.io/gh/Wiiii90/fossredder)

FOSSredder is a desktop application for local expense management targeted at private landlords with multiple properties. The app supports categorization, allocation and structured reporting of expenses. It also supports importing bank statements (PDF) and importing/exporting CSV/XLSX files.

## Features

- Manage multiple properties with custom attributes
- Generate expense reports for a single property or all properties
- Categorize expenses and mark them as allocatable or non-allocatable
- Import and preprocess PDF bank statements for automated candidate extraction
- Optional LLM-assisted suggestion pipeline (adapter-based and configurable)
- User review workflow for verifying, correcting or ignoring import suggestions
- Import/export of CSV and Excel (XLSX)
- Backup and restore functionality
- Analysis tools with graphical charts
- Dark/Light mode
- Local data storage only (no cloud sync)

## Technology stack

- Language: C++20
- GUI: Qt6 (QML)
- Build system: CMake
- Dependency management: vcpkg (manifest mode)
- Persistence: SQLite (runtime storage)
- PDF/OCR: Poppler, Tesseract, OpenCV
- Platform: Windows 10+

## Getting started (overview)

Dependencies are managed via the project `vcpkg.json` manifest. The repository does not include a local copy of `vcpkg`; each developer is expected to have a local `vcpkg` installation and point the build to it via `VCPKG_ROOT` or `CMAKE_TOOLCHAIN_FILE`.

1) Install `vcpkg` once locally:

```powershell
git clone https://github.com/microsoft/vcpkg.git <path-to-vcpkg>
<path-to-vcpkg>\bootstrap-vcpkg.bat
```

2) Set `VCPKG_ROOT` (user environment variable):

```powershell
[Environment]::SetEnvironmentVariable('VCPKG_ROOT', '<path-to-vcpkg>', 'User')
```

If you prefer to control the installed dir explicitly, set `VCPKG_INSTALLED_DIR` similarly.

Configuration notes

- The repository provides `CMakePresets.json` and uses the generator `Visual Studio 18 2026` for the primary presets. The project expects CMake >= 4.0.1. The presets are the source of truth for common workflows.
- On first configure, `vcpkg` will install the manifest dependencies referenced in `vcpkg.json`.

Configure app (example):

```powershell
cmake --preset app
```

Configure tests (example):

```powershell
cmake --preset tests
```

Build examples:

```powershell
cmake --build --preset release-app
cmake --build --preset debug-tests
```

## Quality workflow

The repository ties the quality tooling to CMake presets (`ci`, `tidy`, `coverage`). See `CMakePresets.json` for available presets.

## Documentation

- `docs/` contains design artifacts and requirements. The implementation (code) should be considered the authoritative source when documentation and code disagree.

## License

<<<<<<< HEAD
This project is licensed under the MIT License. See `LICENSE` for details.
=======
```powershell
.\ci\build-debug.ps1 -RunTests
```

Das Skript prüft vor dem Konfigurieren, ob `VCPKG_ROOT` korrekt gesetzt ist.

## Qualitäts-Workflow

Die Qualitätsroutine ist an die CMake-Presets (`ci`, `tidy`, `coverage`) gekoppelt. Lokal und im CI werden dieselben Einstiegspunkte verwendet:

- `ci` / `release-ci` für den vollständigen Release-Testbuild
- `tidy` / `release-tidy` für `clang-tidy` auf den First-Party-Targets
- `coverage` / `release-coverage` für LLVM-Coverage mit `clang-cl`

Die vorhandenen C++-, GTest-, Qt- und QML-Tests bleiben dabei nicht außen vor: Der Coverage-Lauf baut die bestehenden Test-Executables instrumentiert und führt anschließend den kompletten registrierten `ctest`-Satz aus.

### Lokale Qualitätsläufe

**Alle Release-Tests über das CI-Preset bauen und ausführen:**

```powershell
cmake --preset ci
cmake --build --preset release-ci
ctest --preset release-ci
```

**`clang-tidy` lokal ausführen:**

```powershell
cmake --preset tidy
cmake --build --preset release-tidy
```

**Coverage lokal erzeugen:**

```powershell
cmake --preset coverage
cmake --build --preset release-coverage
.\ci\coverage-windows.ps1 -BuildDir .build\coverage -Config Release -OutDir coverage
```

Der Coverage-Lauf erzeugt unter `coverage/`:

- `coverage-summary.txt` mit der LLVM-Zusammenfassung
- `coverage.lcov` für Codecov
- `html/` mit einer Startseite und pro Test-Executable separaten HTML-Reports

Die Tool-Konfigurationsdateien liegen absichtlich an den von den Tools erwarteten Standardorten:

- `.clang-tidy` im Repository-Root für automatische `clang-tidy`-Erkennung
- `codecov.yml` im Repository-Root für die Standard-Erkennung durch Codecov
- `.github/workflows/quality.yml` unter `.github/workflows/` für GitHub Actions

### GitHub-Präsentation

Der Workflow `.github/workflows/quality.yml` läuft bewusst auf einem `self-hosted` Windows-Runner. Das passt zu diesem Projekt besser als GitHub-hosted Runner, weil die native Windows-Toolchain, `Visual Studio 18 2026`, `clang-cl` und eine bereits eingerichtete `VCPKG_ROOT`-Installation dauerhaft auf der Maschine vorhanden sein sollen.

Der Workflow bootstrapped `vcpkg` daher nicht in GitHub Actions, sondern prüft nur die vorhandenen Voraussetzungen des Runners und führt danach dieselben Presets wie lokal aus. Dadurch bleiben lokale und CI-Läufe deckungsgleich, ohne bei jedem Push erneut die komplette Dependency-Kette auf einem ephemeren Runner aufzubauen.

Für den Runner gilt deshalb vor dem ersten CI-Lauf einmalig:

- `cmake` in Version `4.0.1` oder neuer muss installiert sein
- `VCPKG_ROOT` muss dauerhaft auf die lokale `vcpkg`-Installation zeigen
- für den `tidy`-Job muss `clang-tidy` verfügbar sein
- für den Coverage-Job müssen `clang-cl`, `llvm-profdata` und `llvm-cov` verfügbar sein

Die Workflow-Ausführung ist auf Pushes nach `master` sowie auf `workflow_dispatch` beschränkt. Die Badges im README hängen direkt an:

- dem GitHub-Workflow-Status für Build/Test/Tidy/Coverage
- dem Codecov-Upload auf Basis von `coverage/coverage.lcov`

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

This project is licensed under the MIT License. See `LICENSE` for details.

Dieses Projekt ist unter der MIT-Lizenz lizenziert. Weitere Details finden Sie in der [LICENSE](./LICENSE)-Datei.
>>>>>>> d851148 (Refactor quality workflow to self-hosted runner)
