# Contributing to FOSSredder

Thank you for your interest in contributing! This document provides guidance for setting up your development environment and following the project's contribution process.

## Table of Contents

1. [Development Environment](#development-environment)
2. [Build & Test Workflow](#build--test-workflow)
3. [Code Style](#code-style)
4. [Branching & Commit Conventions](#branching--commit-conventions)
5. [Pull Request Process](#pull-request-process)
6. [Architecture Decisions](#architecture-decisions)

---

## Development Environment

FOSSredder targets Windows 10+ with a specific toolchain. Before contributing, ensure you have the following installed:

| Tool | Minimum Version | Notes |
|---|---|---|
| CMake | 4.0.1 | Used for build configuration and presets |
| Visual Studio | 2026 (MSVC 17) | Required for the `Visual Studio 18 2026` generator |
| vcpkg | latest | Manifest mode; bootstrap as described in the README |
| Clang / clang-cl | 18+ | Required for the `tidy` and `coverage` presets |
| Inno Setup | 6+ | Only needed for packaging (`release-package`) |

Follow the **Getting Started** section in [README.md](README.md) for step-by-step setup instructions including vcpkg bootstrapping.

---

## Build & Test Workflow

The project uses CMake presets (see `CMakePresets.json`). The most common commands are:

```powershell
# Configure and build for regular development
cmake --preset app
cmake --build --preset debug-app

# Run the full test suite
cmake --preset tests
cmake --build --preset release-tests
ctest --preset release-tests --output-on-failure

# Run static analysis (requires clang-tidy)
cmake --preset tidy
cmake --build --preset release-tidy

# Generate coverage report (requires clang / clang-cl)
cmake --preset coverage
cmake --build --preset release-coverage
ctest --preset release-coverage --output-on-failure
```

> **Tip:** The CI workflow (`.github/workflows/quality.yml`) runs the `ci` preset, followed by `clang-tidy` and `coverage` jobs on a self-hosted Windows runner. Running these locally before pushing is encouraged.

---

## Code Style

- **Language:** C++20 — no language extensions (`CMAKE_CXX_EXTENSIONS OFF`).
- **Formatting:** Follow existing file formatting conventions. The project uses clang-tidy (see `.clang-tidy`) for static analysis. Fix all tidy warnings before submitting a PR.
- **Documentation:** All public headers must use Doxygen-style comments (`@brief`, `@param`, `@return`, etc.). Run `doxygen Doxyfile` in the project root to verify.
- **Naming:** Follow the conventions already present in each layer (`PascalCase` for classes, `camelCase` for member variables and functions, `k` prefix for constants).
- **New dependencies:** Add them to `vcpkg.json` and verify they compile in CI.

---

## Branching & Commit Conventions

- Branch from `master` for new features or fixes.
- Suggested branch naming: `feature/<short-description>`, `fix/<short-description>`, `chore/<short-description>`.
- Write clear, imperative commit messages: `Add fuzzy matching to ImportMatcher`, `Fix null dereference in StorageManager`.
- Keep commits focused — one logical change per commit.

---

## Pull Request Process

1. Ensure the full test suite passes locally (`ctest --preset release-tests`).
2. Ensure clang-tidy produces no new warnings (`cmake --build --preset release-tidy`).
3. Update documentation (README, Doxygen headers, `docs/`) if behaviour changes.
4. For significant design changes, add or update an ADR in `docs/adr/` (see below).
5. Open a PR against `master` with a clear description of what was changed and why.
6. At least one approval is required before merging.

---

## Architecture Decisions

Significant technical decisions are tracked as Architecture Decision Records (ADRs) in [`docs/adr/`](docs/adr/).

- Use the template in `docs/adr/0000-template.md` when adding a new ADR.
- Number ADRs sequentially (`0001-`, `0002-`, …).
- Set the status to `Proposed` initially; update to `Accepted` or `Superseded` as appropriate.

If you are unsure whether your change warrants an ADR, err on the side of writing one — it makes future contributors' lives easier.
