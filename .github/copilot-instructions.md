# Copilot Instructions

## Documentation Guidelines
- Use clear, concise language.
- Follow the structure and formatting conventions of existing documentation.
- Ensure technical accuracy and consistency with the codebase.
- Always use English language for documentation except in REQUIREMENTS_DE.md where German is primary.

## File and Folder Management
- Prefer using existing files and folder structures first; avoid creating new ad-hoc folders or files when an existing file can be reused or reorganized.

## Code Integration
- Prefer using centralized constants like `QmlContracts.h` to avoid magic strings and keep QML/C++ integration clean.
- Use existing payload abstractions (PayloadKeys, PayloadMapper, EntityPayloadMapper) and broader `ui/include/ui` + `ui/src` structure instead of concentrating new logic in `StateFacade/StateFacadeProjection`.
- Avoid fallback code paths in QML/UI unless explicitly requested; the user considers broad UI fallbacks dangerous and prefers deterministic behavior over fallback behavior.

## Build Management
- Run builds using repository CMake presets (e.g., `cmake --build --preset debug-app`) instead of ad-hoc build commands.

## Cleanup Preferences
- Perform larger cleanup batches instead of very small incremental batches to maintain efficiency and coherence in the codebase.

## Planning Execution
- When creating a plan for this repository, always execute the plan in the same response instead of stopping after the plan announcement.

## Issue Resolution
- When a user says an issue is already resolved, do not rollback broadly; continue strictly with the user’s explicit numbered follow-up tasks.

## Implementation Preferences
- Address only specific numbered points when referenced by the user; do not revisit omitted points.
- Strive for clean deterministic implementations without fallback or redundant code paths.
