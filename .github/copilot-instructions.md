# Copilot Instructions

## Project Guidelines
- User prefers concrete, project-specific analysis rather than generic suggestions; wants me to inspect the actual repository files and give actionable findings.
- User allows API-breaking changes when they materially improve a proper production-grade refactor and avoid interim designs. User does not want backward compatibility or transition paths for this refactor; prefers the clean end state even if it is breaking and removes legacy behavior.
- User allows breaking up and restructuring when it is needed for a proper refactor.
- User validates the app in the Release build because only the Release Qt setup is runnable right now; Debug exe is currently not usable, so runtime fixes should be checked against Release.
- User expects build validation to be reported only after the reported compiler errors are actually fixed, while the broader audit should continue in the same pass.
- User prefers bundled completion of multiple remaining refactor blocks in one pass instead of stopping after a single cleanup step.
- User prefers removing dead or redundant mechanisms completely where prior refactors made them obsolete, and wants Release builds run until green for validation.
- User wants aggressive cleanup of obsolete leftovers, including deleting dead files, empty directories, unused functions, and false duplicates.
- User wants touched files fully formatted, aggressive cleanup of obsolete leftovers and false duplicates, and Doxygen-style documentation comments added broadly for files and relevant functions when touching code.
- UI-related types and helper files should not carry 'ui' prefixes in their names; instead, a `UI` namespace should be used where appropriate.
- Avoid magic strings/numbers by centralizing constants/configuration for maintainability and professional production code.
- User wants visual/layout values to be theme-driven instead of per-view constant modules.
- User does not want fallback variables in UI constants or themes.
- User wants Doxygen-style file comments where sensible and does not want small inline comments unless absolutely necessary. For Doxygen, follow best practice rather than repo drift: prefer meaningful header-first documentation, use tags like `@deprecated`, and avoid redundant source-file documentation.
- User wants any touched code formatted and cleaned up of unnecessary comments, and new tests added when structurally useful.
- User uses pch.h in core and wants all unused includes to be removed and used added to the pch.h.
- User wants not to read in third_party folder files and to ignore them in the analysis.
- User wants vcpkg folder to be ignored on file audits.

## Import Workflow
- User wants the parser subsystem treated as part of the import workflow: parsing may remain a configurable sub-step, but the long-term clean target is to fold the public/private parser folder structure into the import pipeline rather than keep parser as a separate top-level area.