# ADR 0001 – Layered N-Tier Architecture with Dependency Inversion

Date: 2026-01-10

Status: Accepted

## Title

Adopt a layered, modular N-tier architecture with strict dependency inversion as the foundational structural pattern for FOSSredder.

## Context

FOSSredder integrates several heavy third-party libraries (Poppler, OpenCV, Tesseract) that have
large public APIs and are inherently difficult to test in isolation. Early prototypes placed
PDF-rendering and OCR calls directly inside business logic, making unit tests impossible without a
real Tesseract install and real PDF files.

Simultaneously, the Qt/QML UI layer needed to be able to evolve independently of the processing
pipeline — swapping a heuristic parser for a model-based one should not require touching QML views.

The following concerns drove the decision:

1. **Testability** – Domain logic must be unit-testable without instantiating GPU or file-system
   resources.
2. **Replaceability** – The OCR back-end (Tesseract), the PDF renderer (Poppler), and eventually
   the matching strategy must each be replaceable behind a stable interface.
3. **Compile-time isolation** – Long full-project rebuilds slow down iteration. Independent CMake
   targets allow incremental compilation.
4. **Separation of GUI and domain** – Qt-specific code must not leak into `core`; the domain must
   not depend on Qt.

## Decision

Adopt a four-layer architecture mapped directly to CMake targets:

| CMake target | Layer | Responsibility |
|---|---|---|
| `core` | Domain | Business logic, use cases, domain models, repository interfaces |
| `api` | Abstraction | Interfaces for external capabilities (OCR, PDF, image) |
| `services/*` | Infrastructure | Concrete adapters for Poppler, OpenCV, Tesseract |
| `persistence` | Infrastructure | SQLite repositories and schema management |
| `ui` | Presentation | QML views, C++ controller bindings |
| `app` | Composition root | Wires all layers; the only place where concrete types are constructed |

Dependency direction: `ui` → `core` → `api` ← `services`. `core` never imports from `services`,
`persistence`, or `ui`. All infrastructure types reach `core` only through the interfaces in `api`
or `core/include`.

Concretely:

- `core/include/` exports the repository interfaces (`IActorRepository`, `ITransactionRepository`,
  …) that `persistence` implements.
- `api/` exports the service interfaces (`IPopplerAdapter`, `IOpenCvAdapter`,
  `ITesseractAdapter`) that `services/*` implement.
- `app/src/main_qml.cpp` contains the single composition root where factories inject concrete
  implementations.

## Alternatives Considered

| Alternative | Reason for rejection |
|---|---|
| Monolithic single CMake target | No compile-time isolation; mocking impossible without linking everything |
| Service-locator pattern | Hidden dependencies; harder to reason about and test |
| Qt's built-in signal/slot across layers | Couples `core` to Qt types; prevents non-Qt unit tests |
| Plugin-based architecture (shared libraries) | Unnecessarily complex for a single-user desktop app at this stage |

## Consequences

**Positive:**
- `core` unit tests (`core/tests/unit/`) run without Qt, Tesseract, OpenCV, or Poppler; mock
  adapters are injected.
- Swapping the OCR engine only requires a new `services/tesseract` implementation that satisfies
  `ITesseractAdapter`.
- CMake's target-level dependency graph enforces layering at build time; accidental includes are
  caught by the compiler.
- The presentation layer (`ui`) is independently changeable — a future web or CLI front-end could
  reuse `core` without modification.

**Negative / Trade-offs:**
- More boilerplate than a direct call: every capability requires an interface + adapter.
- The composition root (`app/src/main_qml.cpp`) can become large as the number of injected
  services grows; mitigation is factory helpers and the `persistence::Factory`.
- Developers new to the project must understand the layer boundaries before adding code.

## Links

- Architecture diagram: [`docs/DESIGN.md § 3.1`](../DESIGN.md#3-architecture-design-)
- Composition root: `app/src/main_qml.cpp`
- Repository interfaces: `core/include/core/storage/`
- Service interfaces: `api/`
- Example mock usage: `core/tests/unit/TestStorageManager.cpp`

## Author

Wilhelm Altemeier
