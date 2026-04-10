# FOSSredder

[![Quality](https://github.com/Wiiii90/fossredder/actions/workflows/quality.yml/badge.svg?branch=master)](https://github.com/Wiiii90/fossredder/actions/workflows/quality.yml)
[![codecov](https://codecov.io/gh/Wiiii90/fossredder/branch/master/graph/badge.svg)](https://codecov.io/gh/Wiiii90/fossredder)

[TODO: Add BRIEF project description]

## Technology stack

- Language: C++20
- GUI: Qt6 (QML / Qt Quick)
- Build System: CMake
- Dependency Management: vcpkg (manifest mode)
- Persistence: SQLite (runtime storage)
- PDF / Image / OCR: Poppler, OpenCV, Tesseract
- Testing: GoogleTest (`gtest`)
- Logging: `spdlog`
- JSON: `nlohmann-json`
- Packaging: Inno Setup (`installer/fossredder.iss`, `ci/package-inno.ps1`)
- Platform: Windows 10+

## Getting started

### Prerequisites

The project uses CMake (minimum 4.0.1) with the `Visual Studio 18 2026` generator (x64) and `vcpkg` in manifest mode for dependency management.

Install vcpkg (install to a short path, e.g. `C:\vcpkg`):

```powershell
git clone https://github.com/microsoft/vcpkg.git <path-to-vcpkg>
```

Bootstrap vcpkg (prepare vcpkg for use with this repository).

```powershell
<path-to-vcpkg>\bootstrap-vcpkg.bat
```

Set vcpkg root (set `VCPKG_ROOT` to the vcpkg path so CMake can find the toolchain).

```powershell
[Environment]::SetEnvironmentVariable('VCPKG_ROOT', '<path-to-vcpkg>', 'User')
```

The project uses `CMakePresets.json` for presets. List available presets:

```powershell
cmake --list-presets
```

Choose a configure preset and run `cmake --preset <name>` to configure. Examples are given in the upcoming chapters.

### Building

The default workflow configures app and provides debug-app and release-app build variants.

Configure (app preset):

```powershell
cmake --preset app
```

Build debug app (includes debug symbols and has assertions enabled)

```powershell
cmake --build --preset debug-app
```

Executable: `./.build/debug-app/bin/Debug/fossredder.exe`

Build release app (optimized for distribution and packaging):

```powershell
cmake --build --preset release-app
```

Executable: `./.build/release-app/bin/Release/fossredder.exe`

### Testing

The test suite uses GoogleTest and includes unit, interaction, UI/QML, persistence and debug-module tests.

Configure (tests preset):

```powershell
cmake --preset tests
```

Build release tests:

```powershell
cmake --build --preset release-tests
```

Run tests (in console only):

```powershell
ctest --preset release-tests --output-on-failure
```

### Packaging

The project uses Inno Setup for Windows installer packaging through the `release-package` preset.

Build release package (creates the Windows installer via Inno Setup):

```powershell
cmake --build --preset release-package
```

Installer: `./.build/app/dist/fossredder-<version>.exe`

### Quality Workflow

The project uses clang-tidy for static analysis, LLVM coverage for coverage reports, and Codecov for publishing coverage results from the GitHub Actions workflow in `.github/workflows/quality.yml`. The same tools can also be run locally.

Notes: Coverage requires Clang/`clang-cl` and `clang-tidy` runs only if installed and the `tidy` preset is used.

#### CI Workflow

Configure CI (ci preset):

```powershell
cmake --preset ci
```

Build release ci:

```powershell
cmake --build --preset release-ci
```

Run CI tests:

```powershell
ctest --preset release-ci --output-on-failure
```

#### Clang-Tidy

Configure tidy:

```powershell
cmake --preset tidy
```

Build release tidy (runs static analysis during build):

```powershell
cmake --build --preset release-tidy
```

#### Coverage

Configure coverage:

```powershell
cmake --preset coverage
```

Build release coverage (builds the instrumented test binaries):

```powershell
cmake --build --preset release-coverage
```

Run coverage tests (collects coverage data):

```powershell
ctest --preset release-coverage --output-on-failure
```

Coverage output: `./coverage/` and `./coverage/coverage.lcov`.

## Architecture and Design

[TODO: Brief architecture summary and link to design documents as well as design decision ADR files]

## Demo

[TODO: Potential Setup.exe demo AND showcases / screenshots / videos / key features]

## Background

[TODO: Story behind DEV and project motivation]

## Docs

- [TODO: Doxyfiles]
- `docs/` contains design artifacts and requirements. The implementation (code) should be considered the authoritative source when documentation and code disagree.

## License

This project is licensed under the MIT License. See `LICENSE` for details.
