# FOSSredder

[![CI](https://github.com/Wiiii90/fossredder/actions/workflows/quality.yml/badge.svg?branch=master)](https://github.com/Wiiii90/fossredder/actions/workflows/quality.yml)
[![codecov](https://codecov.io/gh/Wiiii90/fossredder/branch/master/graph/badge.svg)](https://codecov.io/gh/Wiiii90/fossredder)

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

The repository ties the quality tooling to the CMake presets `ci`, `tidy`, and `coverage`.

The GitHub workflow in `.github/workflows/quality.yml` runs on the self-hosted Windows runner and is limited to pushes on `master` plus manual `workflow_dispatch` runs.

Local examples:

```powershell
cmake --preset ci
cmake --build --preset release-ci
ctest --preset release-ci
```

```powershell
cmake --preset tidy
cmake --build --preset release-tidy
```

```powershell
cmake --preset coverage
cmake --build --preset release-coverage
.\ci\coverage-windows.ps1 -BuildDir .build\coverage -Config Release -OutDir coverage
```

The coverage run writes reports to `coverage/`, uploads them as the `coverage-report` artifact, and publishes `coverage/coverage.lcov` to Codecov.

## Documentation

- `docs/` contains design artifacts and requirements. The implementation (code) should be considered the authoritative source when documentation and code disagree.

## License

This project is licensed under the MIT License. See `LICENSE` for details.
