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

This project is licensed under the MIT License. See `LICENSE` for details.
