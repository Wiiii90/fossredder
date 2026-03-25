# System Overview and Constraints — FOSSredder

Date: 2026-01-06
Author: Wilhelm Altemeier
Status: Draft

Key Scenario
Bank Statement / Import → Review → Persist → Analyse → Export / Annual Reports

Primary Subsystems
- `app` — Application entrypoint, wiring and lifecycle management.
- `ui` — Qt/QML presentation layer: views, view-models and input validation.
- `core` — Domain logic, use-cases and orchestration services.
- `api` — Internal service interfaces and facades used by UI and core.
- `services/poppler` — PDF rendering and PDF metadata extraction.
- `services/opencv` — Detect tables and preprocess images for OCR.
- `services/tesseract` — OCR text recognition.
- `persistence` — Storage layer: schema management, migrations and backup/restore.
- `debug` — Developer utilities, diagnostics and tooling.

## User Constraints
- Supported architecture: TODO
- Operating system: TODO
- Memory: TODO
- Disk space: TODO
- Permissions: TODO
- Input formats: TODO
- Export formats: TODO
- UI language: English (German planned)

## Developer Constraints
 - Language: C++20
 - CMake: >= 3.15
 - vcpkg: use toolchain file (.../vcpkg/scripts/buildsystems/vcpkg.cmake), manifest mode (vcpkg.json)
