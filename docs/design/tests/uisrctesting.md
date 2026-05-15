# UI Source Testing Matrix

## Purpose

This document defines the target source-layer test matrix for the refactored
`ui` module.

It covers the C++ surface that sits between `core/ports` and QML:

- shell and bootstrap wiring
- workspace orchestration
- import, export, and analysis workflows
- session, selection, navigation, filter, mutation, and status state
- core-facing adapters and payload mappers
- platform services and shared helpers

The goal is to lock down observable behavior and boundary contracts, not
private implementation details.

The matrix is intentionally family-based: collection projection, mutation
reprojection, selection stability, and workflow dispatch should be asserted as
generic rules that apply across all relevant UI collections and views.

## Scope

Included in this matrix:
- `ui/include/ui/*`
- `ui/src/*`
- `app/src/main_qml.cpp` where it wires the UI boundary

Out of scope:
- QML rendering and visual layout
- `core` domain and application behavior
- persistence and infrastructure adapters
- end-to-end flows that cross multiple subsystems at once

## Target Test Tree

```text
ui/
  tests/
    unit/
      shell/
        TestAppContext.cpp
        TestQmlRuntime.cpp
        TestMainWindowContext.cpp
      workspace/
        TestWorkspaceFacade.cpp
      workflows/
        import/
          TestImportWorkflow.cpp
          TestImportWorkflowState.cpp
          TestImportRunStore.cpp
          TestImportJobBridge.cpp
        export/
          TestExportWorkflow.cpp
          TestExportRunner.cpp
          TestWorkspaceSnapshot.cpp
        analysis/
          TestAnalysisWorkflow.cpp
      state/
        session/
          TestWorkspaceSessionState.cpp
          TestWorkspaceSessionModels.cpp
          TestWorkspaceSessionSelection.cpp
        selection/
          TestSelectionState.cpp
        navigation/
          TestNavigationState.cpp
        filters/
          TestFilterState.cpp
        mutation/
          TestSessionMutationState.cpp
        status/
          TestStatusState.cpp
      adapters/
        core/
          TestImportSuggestionService.cpp
          TestAnalysisPayloadMapper.cpp
          TestWorkspaceRowProjector.cpp
          TestAnalysisRequestMapper.cpp
          TestAnalysisResultMapper.cpp
          TestDraftViewMapper.cpp
          TestImportDraftMapper.cpp
          TestImportSuggestionMapper.cpp
          TestEntityPayloadMapper.cpp
          TestPayloadMapper.cpp
      viewmodels/
        system/
          TestSettingsViewModel.cpp
      platform/
        dialogs/
          TestFileDialogs.cpp
        filesystem/
          TestFileSystemBrowser.cpp
        localization/
          TestLanguageService.cpp
        settings/
          TestSettingsStore.cpp
      shared/
        util/
          TestStringConversions.cpp
          TestCoreFacadeGuard.cpp
        payload/
          TestPayloadKeys.cpp
    interaction/
      shell/
        TestMainWindow.cpp
        TestDropHandler.cpp
        TestCloseWorkflow.cpp
      workspace/
        TestWorkspaceSelectionFlow.cpp
        TestWorkspaceRefreshFlow.cpp
      workflows/
        import/
          TestImportState.cpp
        analysis/
          TestAnalysisWorkflow.cpp
        export/
          TestExportWorkflow.cpp
```

## Testing Principles

- Source tests should validate orchestration, mapping, and state transitions.
- Workspace tests should verify that the facade calls the correct core port
  methods and refreshes UI state deterministically.
- Workflow tests should verify run handling, command dispatch, and boundary
  translation for import, export, and analysis.
- Adapter tests should verify stable payload shapes and deterministic mapping.
- State tests should verify selection, navigation, filters, and session
  mutation rules without depending on QML rendering.
- Platform tests should verify OS-facing behavior such as file dialogs,
  filesystem browsing, localization, and settings persistence.
- Shared helper tests should stay narrow and protect only reusable technical
  helpers that are shared across the UI layer.
- When a rule applies to multiple entity families, prefer one generic matrix
  row per behavior family instead of inventing a bespoke case for each screen.

## 1. Shell And Bootstrap

`shell` covers the application boundary, QML registration, main-window wiring,
and the global action surface.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SHL-001 | App context exposes the neutral UI services | Unit | App context initialized | Read the registered services | Workspace facade, workflows, settings, and localization services are available under the final names |
| SHL-002 | QML runtime registers the public UI API only | Unit | QML runtime initialized | Inspect registered types | Only the intended shell, workspace, workflow, viewmodel, and platform types are exposed |
| SHL-003 | Main window context wires actions and services | Unit | Main window context built | Inspect bound services | The context receives the correct workspace, workflow, and status services |
| SHL-004 | Main QML entry wires the root UI boundary | Integration | App bootstrap started | Start the UI entry point | QML sees the same neutral service names that the target architecture defines |
| SHL-005 | Close workflow preserves unsaved changes handling | Interaction | Dirty UI state present | Trigger close | Close prompt or guard is activated instead of silent exit |
| SHL-006 | Drop handler routes file drops into the app boundary | Interaction | File payload provided | Drop files onto the main window | File drop is converted into the expected workspace or import action |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SHL-S-001 | Shell types are the only bootstrap-facing UI surface | Contract | Public headers available | Inspect includes | Bootstrap code depends on shell contracts rather than old controller classes |
| SHL-S-002 | Window helpers stay window-local | Contract | Window source files available | Review include graph | Window helpers do not depend on QML component internals |

## 2. Workspace Boundary

`WorkspaceFacade` is the single UI-facing entry point for catalog-level CRUD,
snapshot access, and selection-friendly read/write orchestration.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| WSP-001 | Workspace facade exposes actor CRUD | Unit | Core ports stubbed | Add, update, and delete an actor | The facade forwards to the correct core operations and refreshes local state |
| WSP-002 | Workspace facade exposes property CRUD | Unit | Core ports stubbed | Add, update, and delete a property | Property operations flow through the same boundary contract |
| WSP-003 | Workspace facade exposes contract CRUD | Unit | Core ports stubbed | Add, update, and delete a contract | Contract relations are forwarded through the workspace boundary |
| WSP-004 | Workspace facade exposes statement and transaction CRUD | Unit | Core ports stubbed | Add, update, and delete booking data | Statement and transaction mutations stay synchronized |
| WSP-005 | Workspace facade exposes annual and analysis CRUD | Unit | Core ports stubbed | Add, update, and delete reporting data | Annual and analysis mutations flow through the same workspace path |
| WSP-006 | Workspace facade rebuilds row snapshots after mutation | Interaction | Workspace data changed | Refresh the workspace state | The UI receives a fresh row projection without stale ids |
| WSP-007 | Workspace row projection stays deterministic | Unit | Snapshot with stable data | Project rows twice | Both projections produce the same row order and payload |
| WSP-008 | Workspace facade returns read snapshots only | Unit | Core ports stubbed | Read workspace data | Returned data is snapshot-shaped, not raw domain state |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| WSP-S-001 | Workspace facade is the only durable catalog entry point | Contract | UI workspace headers available | Inspect public API | Old entity-specific controller classes are not part of the final UI contract |
| WSP-S-002 | Workspace projection helpers stay UI-side only | Contract | Adapter files available | Review include graph | Core ports stay free of UI projection helpers |

## 3. Workflows

`workflows` contains the actual use-case orchestration for import, export, and
analysis. These components translate UI events into port calls and workflow
state changes.

### Import workflow

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-001 | Import workflow starts a statement import | Unit | Import input configured | Start import | The workflow dispatches the expected port call and records running state |
| IMP-002 | Import workflow cancels the active run | Unit | Running import present | Cancel import | Running state is cleared and the cancel path is forwarded |
| IMP-003 | Import workflow handles finalization | Unit | Draft statement loaded | Finalize draft | Finalization payload is built and passed through the boundary |
| IMP-004 | Import workflow stores run metadata | Unit | Completed import payload available | Persist run | Run metadata is written to the run store |
| IMP-005 | Import job bridge translates job payloads | Unit | Job payload ready | Bridge the job | The bridge produces the expected internal request shape |
| IMP-006 | Import workflow state survives refreshes | Interaction | Selected import file exists | Refresh the workflow state | The current file, path, and draft state remain consistent |

### Export workflow

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-001 | Export workflow starts export with selected payload | Unit | Export payload configured | Start export | Export request is forwarded with the current snapshot and config |
| EXP-002 | Export workflow tracks export runs | Unit | Export in progress | Update state | The workflow state reflects the current run status |
| EXP-003 | Export runner emits a snapshot export payload | Unit | Workspace snapshot present | Run export | The runner produces the export artifact or request payload |
| EXP-004 | Workspace snapshot serializes the boundary data | Unit | Workspace data present | Build snapshot | Snapshot contains the selected read-side data and no raw mutable state |

### Analysis workflow

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-001 | Analysis workflow starts with the current workspace snapshot | Unit | Workspace snapshot ready | Run analysis | The workflow asks the core boundary for the current analysis result |
| ANL-002 | Analysis workflow forwards normalized requests | Unit | Analysis request edited | Trigger update | Request mapper receives normalized analysis data |
| ANL-003 | Analysis workflow forwards normalized results | Unit | Core analysis result ready | Project result | Result mapper produces the expected UI-facing payload |
| ANL-004 | Analysis workflow preserves export and filter state | Interaction | Analysis state changed | Refresh workflow | Existing UI state remains stable unless the workflow explicitly changes it |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| WF-S-001 | Workflows stay use-case focused | Contract | Workflow headers available | Review responsibilities | Import, export, and analysis remain separate concerns |
| WF-S-002 | Workflow helpers stay private to their workflow family | Contract | Workflow sources available | Review include graph | Import helpers do not leak into export or analysis and vice versa |

## 4. State

`state` contains the local UI state needed to keep the workspace, selection,
navigation, filter, mutation, and status behavior deterministic.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ST-001 | Workspace session state stores the current collections | Unit | Session state initialized | Set collections | Actor, property, contract, statement, transaction, annual, and analysis collections are stored consistently |
| ST-002 | Workspace session models mirror session state | Unit | Session state updated | Rebuild models | List models reflect the current session state deterministically |
| ST-003 | Workspace session selection tracks current ids | Unit | Selection state ready | Change selection | The selected actor, property, contract, statement, transaction, annual, or analysis id updates correctly |
| ST-004 | Selection state deduplicates repeated ids | Unit | Duplicate ids provided | Update selection | Duplicate ids are removed and order remains stable |
| ST-005 | Navigation state moves through adjacent rows | Unit | Multiple rows available | Trigger prev/next navigation | Selection moves to the expected adjacent item |
| ST-006 | Filter state normalizes invalid combinations | Unit | Invalid filter input | Apply filters | The filter model resolves to a valid state or rejects the invalid combination safely |
| ST-007 | Session mutation state updates draft data deterministically | Unit | Draft state present | Mutate a field | Draft changes are normalized and stored in a predictable way |
| ST-008 | Status state reflects the latest operation outcome | Unit | Workflow status changed | Update status | Status text and severity are updated without losing the previous operation context |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ST-S-001 | Session state remains UI-local | Contract | State headers available | Review include graph | Session state does not depend on QML rendering internals |
| ST-S-002 | Mutation helpers stay isolated from view code | Contract | State source files available | Review include graph | Mutation logic is not duplicated in viewmodels or workflows |

## 5. Adapters And ViewModels

`adapters` and `viewmodels` bridge core-shaped data into UI-shaped payloads
without recreating domain logic in the UI layer.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ADP-001 | Workspace row projector produces stable rows | Unit | Workspace snapshot ready | Project rows | Row payloads and row order match the source snapshot deterministically |
| ADP-002 | Analysis request mapper keeps request shape stable | Unit | Analysis form input ready | Map request | The mapped request contains the normalized fields expected by the workflow |
| ADP-003 | Analysis result mapper keeps result shape stable | Unit | Analysis result ready | Map result | UI-facing analysis data contains the expected summary and detail fields |
| ADP-004 | Draft view mapper preserves draft editing data | Unit | Draft payload ready | Map view | Draft view state contains the fields needed by the editor and nothing more |
| ADP-005 | Import draft mapper preserves workflow context | Unit | Import draft payload ready | Map draft | The mapper retains source file, statement draft, and run context correctly |
| ADP-006 | Import suggestion mapper preserves suggestion semantics | Unit | Suggestion data ready | Map suggestion | The mapper exposes the same suggestion choices and labels across refreshes |
| ADP-007 | Entity payload mapper preserves QVariant shape | Unit | Entity payload ready | Convert payload | The payload keys and values stay stable for QML consumption |
| ADP-008 | Generic payload mapper preserves helper semantics | Unit | Payload input ready | Convert payload | The helper maps only the intended structural fields |
| ADP-009 | Settings viewmodel exposes saveable UI state | Unit | Settings store stubbed | Mutate settings | The viewmodel reflects the current settings and forwards saves |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ADP-S-001 | Adapter naming stays capability-based | Contract | Adapter headers available | Review public APIs | Mapping classes speak in rows, payloads, and requests rather than old controller names |
| ADP-S-002 | Viewmodels stay QML-facing only | Contract | Viewmodel headers available | Review include graph | Viewmodels do not depend on QML visual components |

## 6. Platform And Shared

`platform` and `shared` contain the supporting services that are not part of
the workspace, workflow, or viewmodel responsibilities.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PLT-001 | File dialogs expose deterministic paths | Unit | Dialog service stubbed | Open or accept a dialog | Returned path matches the expected file dialog contract |
| PLT-002 | File system browser exposes navigation behavior | Unit | Browser initialized | Browse or navigate directories | The current folder, selection, and entries update as expected |
| PLT-003 | Language service exposes the active locale | Unit | Language service initialized | Query current language | The service returns the active language and updates correctly on change |
| PLT-004 | Settings store persists neutral settings names | Unit | Settings store initialized | Save and load settings | The stored values round-trip without depending on old controller names |
| SHR-001 | String conversions remain deterministic | Unit | Representative text input | Convert strings | Conversion helpers return stable text and encoding results |
| SHR-002 | Core facade guard blocks invalid access | Unit | Invalid or null facade input | Guard access | The guard prevents unsafe facade use and returns a safe state |
| SHR-003 | Payload keys remain stable | Contract | Payload key headers available | Inspect key constants | Payload keys do not drift across refactors |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PLT-S-001 | Platform services stay OS-local | Contract | Platform headers available | Review include graph | File dialogs, browser, language, and settings services stay out of core logic |
| PLT-S-002 | Shared helpers stay technical | Contract | Shared headers available | Review include graph | Shared helpers do not accumulate domain rules |
