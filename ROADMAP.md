# Roadmap

## Current baseline

The current baseline already includes:

- import workflows for Commerzbank PDF statements
- export workflows for XLSX and CSV
- plot rendering in the export flow
- a functional persistence layer
- a core architecture based on DDD and clean architecture ideas
- a recently refactored `ui/src` layer aligned more closely with the core
- an active and expanding automated test suite
- multithreading for release use

## Release strategy

The next step is to treat the current state as the first stable, named baseline:

- `0.5.0` is the first real runnable version we want to keep stable
- later versions are planned as roadmap targets, not as immediate GitHub milestones
- patch releases such as `0.5.1` are only relevant if an already released baseline needs a hotfix

## Milestone target: `0.5.0`

`0.5.0` is the baseline freeze milestone. It marks the first version that is stable enough to be treated as a reference point for future work.

The work that belongs to this baseline includes:

- stabilizing the Commerzbank PDF import flow
- stabilizing XLSX and CSV export
- keeping persistence functional without redesigning it yet
- locking in the refactored UI and core integration
- expanding regression coverage for the current baseline
- recording the current architecture and release scope

## Planned roadmap after `0.5.0`

### `0.6.0` - Matching and workflow configuration

This milestone should focus on the matcher and the first useful workflow settings:

- replace the current weak matcher with a production-ready matching pipeline
- add fuzzy matching support
- add Levenshtein-based matching support
- prepare the UI for richer match review
- introduce initial workflow settings for import and export
- add user-facing policy hints for validation rules
- use extracted PDF text as a secondary signal for OCR improvement

### `0.7.0` - Calculation and domain hardening

This milestone should focus on the business logic and the domain model:

- refactor the calculation engine for tax and recoverable cost logic
- add support for reverse adjustments and offsets
- make domain entities and value objects more explicit
- rework domain policies for validation and business constraints
- harden export generation for production use
- expand workflow tests around calculation and export

### `0.8.0` - Packaging, build, CI, and documentation

This milestone should focus on making the project look and behave like a production application:

- replace the current Inno Setup packaging flow with a production installer
- add installer options for OCR models, data paths, and runtime configuration
- normalize the CMake build and packaging configuration
- extend GitHub Actions for release-grade CI
- add automated Doxygen documentation generation to CI
- prepare a public release homepage with versioned downloads

### `0.9.0` - Release hardening and polish

This milestone should focus on release readiness:

- polish the current UI for release readiness
- create contract tests for import and export boundaries
- introduce golden PDF fixtures for statement import tests
- add a pre-import review workflow for transaction extraction
- finalize workflow settings coverage
- complete release documentation and onboarding notes

### `1.0.0` - Production release

This is the production release target:

- prepare the final production release candidate
- verify deployment readiness for January production use
- freeze the release scope

## Post-1.0 expansion

These items are intentionally planned after `1.0.0`:

- scalable import support for additional bank statement templates
- OCR stack evaluation and modernization
- a dedicated persistence architecture refactor cycle
- expanded OCR tuning for Poppler, OpenCV, and Tesseract

## Notes on process

- The active GitHub milestone should stay focused on the current baseline work
- later roadmap items should be pulled into issues only when they become actionable
- detailed release notes and changelog entries can be added when release tagging becomes relevant
