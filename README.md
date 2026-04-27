# FOSSredder

[![CI](https://github.com/Wiiii90/fossredder/actions/workflows/quality.yml/badge.svg?branch=master)](https://github.com/Wiiii90/fossredder/actions/workflows/quality.yml)
[![codecov](https://codecov.io/gh/Wiiii90/fossredder/graph/badge.svg?token=LGALNE53Z6)](https://codecov.io/gh/Wiiii90/fossredder)

**FOSSredder** is a deliberately overengineered Windows desktop application built for a concrete, real-world use case: extracting structured data from PDF bank statements issued by Commerzbank, a major German banking institution, in order to automate the annual allocation of recoverable costs to tenants.

The application replaces a manual, Excel-based workflow with a semi-automated pipeline. It extracts transaction data from PDF statements using OCR and heuristic parsing, followed by a custom matching engine that pre-fills entries by linking them to specific actors, properties, and contracts. A dedicated review interface allows for human-in-the-loop validation, streamlining the process of identifying recoverable costs before the final export back into a structured Excel format.

While the core problem could be addressed with simple scripts or well-known LLMs, this project also serves as an engineering exercise to explore modular system design, extensibility, and modern development workflows:
* **Modular System Design:** Built with decoupled CMake targets (core, persistence, ui, services) to ensure a clear separation of concerns and high maintainability.
* **Extensible Architecture:** Designed to support additional document formats through a strategy-based approach that leverages internal service adapters for OCR and parsing.
* **Modern Development:** Built using an iterative, AI-assisted workflow with a strong emphasis on Clean Architecture principles.
* **Privacy by Design:** Since financial data is sensitive, the system runs strictly locally. No cloud, no external APIs.

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

## Project Status

This application is in **Active Alpha**. The core OCR and processing engine are functional, with current development focused on system stability and specialized financial logic.

* **Core Functionality:** High-performance OCR pipeline, heuristic parsing, and the matching engine for Commerzbank statements are implemented and operational.
* **Current Focus:** Implementing **Tax & Cost Analysis Logic** (allowing users to adjust transaction amounts by tax rates or non-recoverable cost shares) and expanding **Test Coverage**. I am currently stabilizing the UI-to-Persistence pipeline using a TDD approach.
* **In Development:** Finalization of the Excel export module (including tax-adjusted outputs), advanced settings persistence, and UI/UX polishing.

## Architecture and Design

FOSSredder follows a modular N‑tier architecture designed for testability and a clear separation of concerns. The repository is organised into independent targets so core domain logic, persistence, and the UI can be developed and verified in isolation.

### Overview
- **Core (`core`):** Orchestrates the import pipeline and coordinates parsing, matching and export flows. The heavy OCR/processing work is performed by service adapters; `core` composes these steps (see `core/src/import`, `core/src/analysis`).
- **Persistence (`persistence`):** SQLite-backed repositories and DB helpers; includes low‑level wrappers like `StmtGuard.h`. A schema implementation and in‑code migrations exist (`persistence/src/SqliteSchema.cpp`, uses `PRAGMA user_version`), though there is no separate versions table or external migration runner.
- **Services (`services`):** Adapters for external libraries (Poppler, OpenCV, Tesseract). These isolate third-party library interfaces behind small adapters so the core sees a stable interface; adapters are mockable in tests (see `services/poppler`, `services/opencv`, `services/tesseract`).
- **UI (`ui`):** QML front-end and a C++ binding layer. This layer manages presentation logic, controllers, and UI test harnesses (see `ui/src`, `ui/qml`).
- **Composition Root (`app`):** Central runtime bootstrap in `app/src/main.cpp` where concrete implementations and factories are wired together.

### Notable Patterns and Details
- **Domain Model Separation:** Core data models (e.g., `Actor`, `Transaction`) are implemented as lightweight Plain Old C++ Objects (POCOs) within `core/include/core/models`. They remain strictly decoupled from Qt/QML and the persistence layer.
- **Logic-less UI:** QML is primarily used for declarative markup; UI state and business logic run in C++ controllers (see `ui/src/controllers`) and the core. QML files may contain lightweight view glue (loaders, small helper functions or timers) but not domain rules; structured payloads are used instead of magic strings or hardcoded values.
- **Dependency Inversion:** Core logic depends on abstract interfaces (e.g., `core/include/core/storage/IStorageManager.h`). Concrete implementations are provided by the composition root via factory wiring (e.g., `setRepoFactory`). See `core/include/core/storage/IStorageManager.h` for the exact storage API (for example `setRepoFactory`, `setAtomicStoreLoad`, `setAtomicStoreSave`, `saveAs`, `createNew`).
- **Strategy Pattern:** Used for parsing and matching to allow interchangeable heuristics for different bank formats (see `core/src/import` strategies).
- **Manual Dependency Injection:** Uses constructor injection and small factories rather than an automatic DI container to keep wiring explicit and manageable (see `app/src/main.cpp`). The concrete wiring for repo factories and atomic store hooks is performed in `app/src/main.cpp`.
- **Concurrency:** The import pipeline utilizes a bounded worker queue and scheduler for stable throughput and orderly shutdowns (`core/src/jobs/Scheduler.cpp`). See that file for bounded queue behaviour, `stop()` shutdown semantics and exception reporting.
- **Financial Logic:** Exporters and tax/adjustment logic are present but currently undergo refinement regarding rounding rules and locale-specific formatting (`core/src/export`).
- **Error Handling:** Centralized reporting path utilizing `spdlog` sinks (see `core/errors`). A global error reporter is created and installed in `app/src/main.cpp` and Qt messages are forwarded to it via the `qtMessageHandler`.

### Documentation & API Reference
The project maintains a multi-layered documentation strategy:

- **Architectural Decisions (ADRs):** Recorded in `docs/adr/` to track the "why" behind key design choices.
- **Detailed Design:** Found in `docs/DESIGN.md`. Upcoming updates will include component diagrams and explicit export validation rules.
- **Technical API Reference:** All source headers are documented using Doxygen-style comments (`@brief`, `@param`, etc.). You can generate the full HTML reference by running `doxygen Doxyfile` in the project root.
- **Source of Truth:** While we strive for accuracy in our design artifacts, the **implementation (code) remains the authoritative source** in case of discrepancies.

## Roadmap & To-Do

The following milestones highlight both completed foundations and planned refinements:

- [x] **Multi-threaded Pipeline:** Core worker/scheduler system implemented for high-performance batch processing. (see `core/src/jobs/Scheduler.cpp`, `ui/src/import/ImportJobBridge.cpp`)
- [x] **Localization (i18n):** Full infrastructure integrated via `LanguageController` for German/English runtime switching. (see `ui/src/controllers/LanguageController.cpp`, `app/i18n/`)
- [x] **Deployment Baseline:** Basic Windows installer (.exe) workflow established via Inno Setup. (see `installer/fossredder.iss`, packaging preset)
- [ ] **Advanced Matching Logic:** Refactor the matching engine to utilize **Levenshtein distance** (fuzzy matching) for robust OCR data handling — no implementation present yet.
- [ ] **Settings & Configuration:** `Settings` UI parts exist (e.g. `ui/qml/FossRedder/Views/Settings/SettingsExport.qml`) but persistent backend bindings and full settings view implementation are still pending.
- [ ] **Advanced Import Parameters:** Backend adapters for Poppler/OpenCV/Tesseract exist, but UI → backend wiring for fine-grained import parameters is not implemented.
- [ ] **Refine Export Module:** Core exporters (`CsvExporter`, `XlsxExporter`) are implemented (`core/src/export/`), however output formatting, correctness and UI-driven column mapping/template support remain unstable and require work.
- [ ] **Tax & Financial Logic:** Ensure correct application/removal of taxes, rounding and locale rules in exported outputs; decide whether to emit Excel formulas or literal values and implement accordingly. Parts of the tax-related analysis are present (`core/src/analysis/strategies/CalcAnalysisStrategy.cpp`) but export-side handling needs verification (`core/src/export/CsvExporter.cpp`, `core/src/export/XlsxExporter.cpp`).
- [ ] **UI/UX Polishing:** Application menu structure and a dedicated Help/Onboarding flow are not implemented (current About dialog is a placeholder).
- [ ] **Robust Packaging:** Installer skeleton exists but automatic bundling of runtime assets (for example Tesseract training data) and full zero-config packaging are not implemented.
- [ ] **Extended Test Suite:** Unit and some QML tests exist, but end-to-end and broader UI/QML coverage need expansion.

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

**Note:** CMake should pick up the vcpkg toolchain automatically when using these presets because the project calls `fossredder_configure_vcpkg()` in `CMakeLists.txt`. If your environment does not, set the `VCPKG_ROOT` environment variable and re-run the configure preset.

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

Executable: `./.build/app/bin/Debug/fossredder.exe`

Build release app (optimized for distribution and packaging):

```powershell
cmake --build --preset release-app
```

Executable: `./.build/app/bin/Release/fossredder.exe`

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

## Docs

- `docs/` contains design artifacts and requirements. The implementation (code) should be considered the authoritative source when documentation and code disagree.

## License

This project is licensed under the MIT License. See `LICENSE` for details.
