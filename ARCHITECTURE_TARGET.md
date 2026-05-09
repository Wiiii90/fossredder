# FossRedder Architecture Target

## Purpose

This document defines the target architecture for FossRedder. It is meant as a
concrete refactoring guide for this repository, not as a generic Clean
Architecture sketch.

The goal is:

- keep business and application rules in `core`
- keep technical engines and frameworks outside `core`
- remove the `api` layer
- rename technical `services` to `infrastructure`
- keep `import` and `export` workflows in `core/application`
- extract only the technical PDF, OCR, image, and archive engines in the
  first infrastructure slice

## Architectural Decision

### `api` will be removed

The old `api` package was an attempt to let `core` and technical services share
contracts without seeing each other. That creates a third owner for application
contracts. The target architecture does not keep that layer.

Target:

- service ports live in `core/ports/services`
- adapter implementations live outside `core`
- `api` is removed after compatibility includes are gone

### `services` will become `infrastructure`

The current `services/poppler`, `services/opencv`, and `services/tesseract`
folders are not domain services. They are technical adapter implementations.

Target:

- `services/poppler -> infrastructure/poppler`
- `services/opencv -> infrastructure/opencv`
- `services/tesseract -> infrastructure/tesseract`
- `services/archive -> infrastructure/archive`
- no `infrastructure/import` or `infrastructure/export` target in the first
  slice

This rename is about semantics, not just folders. The new module names should
say what they do:

- `infrastructure/poppler` renders or extracts PDF pages
- `infrastructure/opencv` manipulates images and masks
- `infrastructure/tesseract` performs OCR
- `infrastructure/archive` packs files into archives
- `import` and `export` workflows stay in `core/application`

### Domain services stay in `core`

A DDD domain service represents a domain rule that does not naturally belong on
one entity or value object. Examples:

- matching a transaction draft to actors, contracts, and properties
- deciding which contract aliases are relevant
- deriving domain-level import suggestions

These concepts may stay in `core/domain` or `core/application` as policies or
use cases. If the implementation needs technical dependencies such as
Levenshtein, fuzzy search indexes, OCR engine metadata, or installed parser
templates, those details move behind ports and into `infrastructure` in a later
slice.

This means:

- the import use case stays in `core/application/import`
- the export use case stays in `core/application/export`
- the archive writer is a port plus infrastructure implementation
- template catalogs, fuzzy matchers, and package installers are later slices

## Dependency Rule

Allowed direction:

```text
app
  -> ui
  -> persistence
  -> infrastructure
  -> debug
  -> core

persistence      -> core
infrastructure   -> core
ui               -> core
debug            -> core
core             -> standard library and small pure libraries only
```

Forbidden:

- `core -> ui`
- `core -> persistence`
- `core -> infrastructure`
- `core -> api`
- `core -> Qt`
- `core -> SQLite`
- `core -> Poppler`
- `core -> OpenCV`
- `core -> Tesseract`
- `core -> xlnt`
- `core -> libzip`

## SOLID And DDD Fit

### Single Responsibility Principle

Use cases in `core/application` orchestrate one workflow. Technical IO,
rendering, OCR, and archive writing are not application responsibilities and
should be moved to infrastructure adapters.

Examples:

- `ImportStatement` orchestrates import.
- `IPopplerService` renders or extracts PDF data.
- `ITesseractService` performs OCR.
- `IArchivePackager` creates archives.

### Open/Closed Principle

New OCR engines, PDF renderers, image processors, or archive packagers should
be added by implementing ports, not by changing core use cases.

Examples:

- install another OCR package by adding an infrastructure implementation
- add another archive implementation behind the archive port
- add another PDF or OCR adapter behind a port

### Dependency Inversion Principle

`core` owns the interfaces it needs. Infrastructure implements them.

This is why ports belong in `core/ports`, not in `api`.

### DDD Boundary

Domain language belongs in `core/domain`:

- Actor
- Property
- Contract
- Statement
- Transaction
- Analysis
- Annual
- Alias
- matching policies

Technical language belongs in infrastructure:

- PDF rendering
- image masking
- OCR extraction
- ZIP packaging


## Target Repository Layout

```text
app/
  CMakeLists.txt
  src/
    main.cpp
    main_qml.cpp
    Environment.cpp
    Environment.h

core/
  CMakeLists.txt
  include/core/
    application/
      analysis/
      export/
      import/
        draft/
        internal/
        statement/
        transaction/
      storage/
      workspace/
    constants/
    domain/
      entities/
      policies/
      values/
    errors/
    jobs/
    ports/
      export/
      import/
      presenters/
      repositories/
      services/
      storage/
    utils/
    pch.h
  src/
    application/
      analysis/
      export/
      import/
      storage/
      workspace/
    domain/
    errors/
    jobs/
    utils/
  tests/

infrastructure/
  CMakeLists.txt
  archive/
    include/infrastructure/archive/
    src/
    tests/
  opencv/
    include/infrastructure/opencv/
    src/
    tests/
  poppler/
    include/infrastructure/poppler/
    src/
    tests/
  tesseract/
    include/infrastructure/tesseract/
    src/
    tests/

persistence/
  CMakeLists.txt
  include/persistence/
    repositories/
  src/
    repositories/
  tests/

ui/
  CMakeLists.txt
  include/ui/
    analysis/
    bootstrap/
    controllers/
    dialogs/
    export/
    import/
    models/
    observability/
    payload/
    state/
    text/
    util/
    window/
  qml/
  src/
  tests/

debug/
  CMakeLists.txt
  include/debug/
  src/

docs/
installer/
ci/
cmake/
```

Removed target layout:

```text
api/
services/
```

## Target CMake Targets

```text
FossRedder::Core
FossRedder::Persistence
FossRedder::InfrastructureArchive
FossRedder::InfrastructureOpenCv
FossRedder::InfrastructurePoppler
FossRedder::InfrastructureTesseract
FossRedder::Debug
FossRedder::Ui
fossredder
```

Optional simplification:

```text
FossRedder::Infrastructure
```

This single target may replace the smaller infrastructure targets if separate
build/test boundaries are not useful. Even then, infrastructure code stays
outside `core`.

Removed target:

```text
FossRedder::Api
```

## Use Case Split

### Import

Keep the import workflow in `core/application/import`.

Responsibilities:

- coordinate page rendering, masking, OCR, and parsing
- compose an import request into page work
- produce import results and logs
- decide what to do with the parsed drafts

Technical substeps:

- render PDF pages
- mask images
- run OCR
- load statement templates, if needed, through a later slice
- fuzzy-match text and metadata, if needed, through a later slice

For the first target slice, only the PDF, image, and OCR engines are extracted
to `infrastructure`. Template catalogs, OCR package installation, and fuzzy
matching remain core concerns until the later slice that needs them.

### Export

Keep the export workflow in `core/application/export`.

Responsibilities:

- coordinate export requests
- select export formats
- gather analysis/workspace data
- produce export results and logs

Technical substeps:

- package archive files
- encode assets or export attachments, if needed, through a later slice

For the first target slice, only archive packaging is extracted to
`infrastructure`. Workbook and CSV writers remain in core or a later slice.

## Core Target Shape

### Domain

Keep in `core/include/core/domain`:

- `entities/Actor.h`
- `entities/Analysis.h`
- `entities/Annual.h`
- `entities/Contract.h`
- `entities/Property.h`
- `entities/Statement.h`
- `entities/Transaction.h`
- `values/Alias.h`
- `values/AliasUsage.h`
- `policies/DraftMatchingPolicy.h`

Domain code may use standard library types and pure value objects. It must not
know about OCR, image processing, PDF engines, databases, Qt, or filesystems
unless a value object explicitly represents a domain concept.

### Application

Keep in `core/include/core/application`:

- workspace/session orchestration
- import use cases and parser workflow models
- analysis use cases
- export use cases
- storage orchestration over repository/storage ports
- request/result types

Application code may depend on `core/domain` and `core/ports`.

### Ports

Keep in `core/include/core/ports`:

- `repositories/*`
- `storage/*`
- `presenters/*`
- `services/IOpenCvService.h`
- `services/IPopplerService.h`
- `services/ITesseractService.h`
- `services/OpenCv*.h`
- `services/Poppler*.h`
- `services/Tesseract*.h`
- `export/IArchivePackager.h`

Add as needed in later slices:

- `import/IImportTemplateCatalog.h`
- `import/IStatementParserTemplate.h`
- `import/IOcrPackageInstaller.h`
- `import/ITextMatcher.h`
- `export/IXlsxWorkbookWriter.h`
- `export/ICsvWriter.h`
- `export/IExportArtifactWriter.h`

### Suggested Concrete Ports

Use these when extracting the remaining technical steps:

- `core/ports/export/IArchivePackager.h`
- `core/ports/services/IOpenCvService.h`
- `core/ports/services/IPopplerService.h`
- `core/ports/services/ITesseractService.h`

Later slices may add:

- `core/ports/import/IImportTemplateCatalog.h`
- `core/ports/import/IImportTemplateInstaller.h`
- `core/ports/import/IStatementTemplateParser.h`
- `core/ports/import/ITextMatcher.h`
- `core/ports/export/IXlsxWorkbookWriter.h`
- `core/ports/export/ICsvWriter.h`
- `core/ports/export/IExportArtifactWriter.h`

## Infrastructure Target Shape

### PDF, OCR, Image Processing

Move current technical adapters:

- `services/poppler/* -> infrastructure/poppler/*`
- `services/opencv/* -> infrastructure/opencv/*`
- `services/tesseract/* -> infrastructure/tesseract/*`

These modules implement:

- `core::ports::services::IPopplerService`
- `core::ports::services::IOpenCvService`
- `core::ports::services::ITesseractService`

Suggested target names:

- `FossRedder::InfrastructurePoppler`
- `FossRedder::InfrastructureOpenCv`
- `FossRedder::InfrastructureTesseract`

### Archive

Move archive packaging out of `core`:

- ZIP packaging using `libzip`
- concrete archive filesystem work
- export attachment packaging

Target infrastructure files:

```text
infrastructure/archive/
  include/infrastructure/archive/ZipArchivePackager.h
  src/ZipArchivePackager.cpp
  tests/
```

Core keeps the export use case and the archive port. Infrastructure owns the
archive layout, filesystem work, and zip library integration.

Suggested target names:

- `FossRedder::InfrastructureArchive`

## Debug Layer

`debug` remains outside `core`.

Target role:

- implement diagnostic/reporting adapters
- provide runtime tracing tools
- implement or consume `core/errors/IErrorReporter`

Target structure:

```text
debug/
  include/debug/
    ErrorReporter.h
    FileDebugger.h
    IDebugger.h
    RunManager.h
    SpdlogDebugger.h
  src/
    ErrorReporter.cpp
    FileDebugger.cpp
    RunManager.cpp
    SpdlogDebugger.cpp
```

Debug may depend on `core`. `core` must not depend on `debug`.

## Test Matrix

Testing should follow the same module boundaries.

### `core`

Test:

- domain policies
- application use cases
- request/result mapping
- composition logic that is still pure enough to keep in core

Location:

- `core/tests`

### `infrastructure`

Test:

- OCR adapter behavior
- PDF adapter behavior
- archive writer behavior
- filesystem packaging behavior
- adapter boundary error handling

Location:

- `infrastructure/*/tests`
- specifically `infrastructure/archive/tests`

### `persistence`

Test:

- repository mappings
- SQLite schema behavior
- registry persistence

Location:

- `persistence/tests`

### `ui`

Test:

- controllers
- mappers
- state projections
- QML behavior where useful

Location:

- `ui/tests`

### `debug`

Test:

- report formatting
- reporter wiring
- runtime helper behavior

Location:

- `debug/tests` if needed

## Persistence Layer

`persistence` remains separate from generic infrastructure because it is already
a cohesive adapter module with SQLite repository implementations.

Target role:

- implement `core/ports/repositories`
- implement `core/ports/storage`
- own SQLite schema and transactions

`persistence` may depend on SQLite and `nlohmann_json`. `core` should not depend
on SQLite.

## UI Layer

`ui` remains the inbound adapter layer.

Target role:

- controllers collect UI input
- mappers translate UI payloads to application requests
- models expose UI-friendly state
- QML and Qt live here

UI may depend on `core`, `debug`, and Qt. `core` must not depend on UI or Qt.

## App Composition Root

`app` wires the final object graph:

- UI controllers
- persistence repositories
- infrastructure adapters
- debug reporters
- core use cases and facades

The app target is the correct place to choose:

- which OCR engine is active
- where Tesseract packages are installed
- which archive packager is registered
- which PDF renderer and image adapter implementations are active

## Migration Plan

1. Finish replacing `api/*` usage in `core` with `core/ports/*`.
2. Remove `FossRedder::Api` from `core/CMakeLists.txt`.
3. Replace remaining `api` includes in `app`, `services`, and tests with
   `core/ports/*` or infrastructure headers.
4. Rename `services` to `infrastructure` in folders, namespaces, targets, and
   CMake aliases.
5. Move archive packaging out of `core/application/export` into
   `infrastructure/archive`.
6. Add `infrastructure/archive/tests` and keep archive behavior covered there.
7. Keep `import` and `export` workflows in `core/application` for now.
8. Defer template catalogs, fuzzy matching, and workbook or CSV writer
   extraction until the slice that actually needs them.
9. Delete `api/` and `FossRedder::Api`.
10. Run preset-based target builds after each step.

If a step is large, do it in this order:

1. add the port
2. add the infrastructure implementation
3. switch one call site
4. update tests
5. remove the old compatibility include

## Build Guidance

Prefer presets and narrow targets.

Examples:

```powershell
cmake --preset <preset-name>
cmake --build --preset <build-preset-name> --target fossredder_core
cmake --build --preset <build-preset-name> --target fossredder_opencv_service
```

Avoid full app builds unless the changed boundary requires end-to-end linking.

## Current Transitional State

- Some service DTOs already exist in `core/ports/services`.
- Some `api/*` headers are compatibility wrappers.
- `core` still has some includes that should be moved from `api/*` to
  `core/ports/*`.
- `services` still exists on disk and in CMake.
- archive packaging still lives partly in `core`.

This state is temporary. The target architecture removes `api` and treats
technical engines as infrastructure adapters while keeping import and export
as application workflows for now.
