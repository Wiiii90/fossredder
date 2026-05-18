# UI Source Testing Matrix

## Purpose

This document defines the current target test matrix for the `ui/src` C++
surface.

It mirrors the refactored UI boundary and keeps the source tests aligned with
the real code in `ui/tests`. The goal is to lock in the behavior of the UI
bridge layer:

- workspace facade and selection wiring
- session state and derived collections
- row projection and selection helpers
- mutation normalization helpers
- neutral navigation state
- thin adapter smoke checks
- small workflow smoke checks

The matrix is intentionally family-based. When the same rule applies to all
workspace collections, selection helpers, or row projection helpers, the test
should describe that family instead of a single incidental screen-specific
example.

## Scope

Included in this matrix:
- `ui/tests/unit/*`
- `ui/tests/interaction/*`
- `ui/tests/support/*`
- the source layer in `ui/include/ui/*` and `ui/src/*` that these tests cover

Out of scope:
- QML rendering and visual layout
- `core` domain and application behavior
- persistence and infrastructure adapters

## Target Test Tree

```text
ui/
  tests/
    support/
      FakeStorageManager.h
      WorkspaceTestData.h
    unit/
      TestAnalysisPayloadMapper.cpp
      TestAnalysisWorkflow.cpp
      TestExportWorkflow.cpp
      TestImportSuggestionService.cpp
      TestImportWorkflow.cpp
      TestNavigationState.cpp
      TestSelectionState.cpp
      TestSessionModels.cpp
      TestSessionMutationState.cpp
      TestWorkspaceFacade.cpp
      TestWorkspaceStartupRehydration.cpp
      TestWorkspaceRowProjector.cpp
      TestWorkspaceSessionSelection.cpp
      TestWorkspaceSessionState.cpp
    interaction/
      TestImportState.cpp
```

## Testing Principles

- Source tests should validate observable behavior, not private helper details.
- Workspace tests should verify projection, selection, and boundary routing.
- Session tests should verify that collections stay consistent across refresh
  and deletion impact.
- Projection refresh tests should verify that derived rows rebind when the
  workspace revision changes.
- Row projection tests should verify stable ids, stable ordering, and stable
  selection behavior across all workspace families.
- Ordered selection projection should expose a flat row contract for QML:
  `rows`, `orderIds`, `index`, `id`, and `currentId` should stay in sync with
  the nested `selection` payload.
- Adapter smoke tests should keep the payload surface and mapper headers
  usable, even when the implementation remains intentionally thin.
- Workflow smoke tests should keep the import and export boundaries usable
  without coupling the source tests to QML rendering.
- Prefer family-based assertions over screen-specific one-off cases when the
  same rule applies to multiple collections.

## 1. Workspace Boundary

`WorkspaceFacade` is the UI-facing entry point for workspace projection,
selection access, and core-boundary mutation routing.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| WSP-001 | Workspace projection loads all catalog families | Unit | Facade with a representative workspace catalog | Load the catalog into the facade | Actor, property, contract, statement, transaction, analysis, and annual rows are all available |
| WSP-002 | Selection accessors resolve current workspace rows | Unit | Facade with a representative workspace catalog | Set actor, property, contract, statement, transaction, analysis, and annual selection ids | The selected rows resolve to the expected objects |
| WSP-003 | Statement transaction ids stay readable through the facade | Unit | Facade with a representative statement | Query statement transaction ids | The facade returns the current ordered transaction id list |
| WSP-004 | Workspace path changes propagate through the boundary | Unit | Facade wired to a storage manager stub | Create or open a workspace path | The facade and storage manager agree on the current path |
| WSP-005 | Mutations route through the core boundary and refresh rows | Unit | Facade wired to a writable core boundary | Add and delete actors | The actor row model updates deterministically after each mutation |
| WSP-006 | Workspace revision changes rebind derived row projections | Unit | Facade with a representative workspace catalog and a visible derived row consumer | Mutate the workspace and bump the revision | Statement, transaction, and other derived collection bindings re-evaluate deterministically |
| WSP-007 | Startup rehydrates persisted workspace families into UI models | Unit | Facade attached to a loaded workspace session state with workflow logs | Open the workspace and refresh the facade state | Booking, analysis, import, and export rows are restored from persisted state without dropping relations |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| WSP-C-001 | Workspace facade stays the public workspace entry point | Contract | UI workspace headers available | Review public API | The facade exposes workspace behavior without reintroducing old controller classes |
| WSP-C-002 | Workspace projection stays source-side | Contract | Workspace source files available | Review include graph | Projection helpers remain in the UI source layer and do not leak into core |

## 2. Session State

`SessionModels`, `SessionSelection`, `SelectionState`, `NavigationState`, and
`SessionMutationState` keep the UI workspace state deterministic.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ST-001 | Session models project every workspace collection family | Unit | Session loaded from a representative workspace catalog | Rebuild the models | Actor, property, contract, statement, transaction, analysis, and annual models all match the source catalog |
| ST-002 | Session models refresh derived transaction types after contract changes | Unit | Catalog with a changed contract type | Reload the models | Transaction types reflect the updated contract type |
| ST-003 | Session state exposes filtered transaction views for statements and properties | Unit | Session loaded from a representative workspace catalog | Query statement and property filters | Statement and property transaction filters return the expected counts |
| ST-004 | Session state applies deletion impact across collections and filters | Unit | Session loaded from a representative workspace catalog | Apply statement, transaction, and property deletion impact | The affected collections and filtered views are cleared consistently |
| ST-005 | Session state updates transaction property assignments in place | Unit | Session loaded from a representative workspace catalog | Reassign transaction property ids | The transaction row updates and the property filter shrinks accordingly |
| ST-006 | Session selection tracks current rows across all collections | Unit | Session models with a representative workspace catalog | Select actor, property, contract, statement, transaction, analysis, and annual ids | Each selected row resolves to the expected object |
| ST-007 | Session selection clears stale selections after reload | Unit | Session models with selected rows | Remove the backing rows and refresh selection | Stale ids and stale analysis result state are cleared deterministically |
| ST-008 | Selection state projects and clears rows consistently | Unit | Selection state with a representative workspace catalog | Set and then remove source rows | Selected rows resolve while present and clear once the source disappears |
| ST-009 | Navigation state stores the current section and settings category | Unit | Fresh navigation state | Change section and settings category values | Section and category values stay in sync with the enum representation |
| ST-010 | Mutation helpers normalize string and draft state families | Unit | Mixed string collections and draft payloads | Normalize, insert, remove, and current-state helpers | String collections, transaction drafts, and draft-list helpers stay deterministic |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ST-C-001 | Session state remains UI-local | Contract | Session source files available | Review include graph | Session logic stays inside the UI source layer |
| ST-C-002 | Mutation logic stays reusable across collections | Contract | Mutation helper source available | Review helper surface | Shared mutation helpers are used instead of duplicating collection-specific logic |

## 3. Row Projection And Adapters

`WorkspaceRowProjector` and the thin mapper smoke tests protect the row and
payload shapes that QML and workflows consume.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ADP-001 | Workspace row projection stays stable for all workspace families | Unit | Session store loaded from a representative catalog | Build actor, property, contract, analysis, annual, statement, and transaction rows | Each family exposes deterministic ids, names, and role payloads |
| ADP-002 | Workspace row ordering preserves the generic selection contract | Unit | Synthetic row list with ordered ids | Insert, prune, and reorder ids | Ordering, insertion, wrapping, and reselection behave deterministically, and the ordered selection payload exposes stable `orderIds`, `index`, `id`, and `currentId` fields |
| ADP-003 | Analysis payload mapper header stays usable | Unit | Mapper header available | Include the mapper header | The adapter remains a valid build-time contract surface |
| ADP-004 | Import suggestion mapper header stays usable | Unit | Mapper header available | Include the mapper header | The adapter remains a valid build-time contract surface |
| ADP-005 | Analysis preview returns filtered transactions from the current workspace snapshot | Unit | Analysis workflow with a representative workspace catalog | Request a preview for an empty filter and for a matching filter expression | The preview returns the visible transactions, the count metrics, and the row payloads stay aligned with the workspace snapshot |
| ADP-006 | Analysis filter composition defaults to the previous year and skips unrestricted groups | Unit | Analysis workflow with UI filter inputs and settings defaults | Build a filter from an unconfigured year selector and from all-selected property and contract groups | The canonical filter uses the previous year as default, keeps the year mode as the default case, and omits property and contract clauses when the selection is unrestricted |
| ADP-007 | Analysis filter composition keeps explicit partial selections intact | Unit | Analysis workflow with UI filter inputs | Build a filter from a partial property selection, a partial contract selection, and an explicit allocatable mode | The canonical filter preserves the explicit clauses and remains stable for preview and execution |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ADP-C-001 | Row projector stays a UI source helper | Contract | Adapter headers available | Review public API | Row projection helpers stay capability-based and do not drift back into controller naming |
| ADP-C-002 | Thin mappers stay header-usable | Contract | Mapper headers available | Compile the smoke tests | The mapper headers remain included and usable without runtime coupling |

## 4. Workflow Smokes

The workflow unit tests keep the export and import boundaries usable without
binding the suite to QML rendering.

### ExportWorkflow

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-W-001 | Restored export runs are projected from persisted state | Unit | Snapshot provider with export logs | Construct the workflow | Restored export runs are visible in the workflow model |
| EXP-W-002 | Snapshot refresh replaces export run rows deterministically | Unit | Workflow with restored runs and a changed snapshot | Refresh from the state snapshot | Workflow rows rebind to the new snapshot content |
| EXP-W-003 | Log-store hookup does not overwrite restored export runs | Unit | Workflow with restored runs and a writable log store | Bind the log store and mutate a run | Restored rows remain intact until a user mutation occurs |

### ImportWorkflow

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-W-001 | Restored import runs are projected from persisted state | Unit | Snapshot provider with import logs | Construct the workflow | Restored import runs are visible in the workflow model |
| IMP-W-002 | Snapshot refresh replaces import run rows deterministically | Unit | Workflow with restored runs and a changed snapshot | Refresh from the state snapshot | Workflow rows rebind to the new snapshot content |
| IMP-W-003 | Log-store hookup does not overwrite restored import runs | Unit | Workflow with restored runs and a writable log store | Bind the log store and mutate a run | Restored rows remain intact until a user mutation occurs |

## 5. Import Interaction

The interaction smoke test keeps the import workflow boundary usable without
binding the suite to QML rendering.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-001 | Import workflow header remains usable | Interaction | Import workflow state header available | Include the workflow state header | The import workflow boundary remains buildable and accessible from the UI source layer |

### Boundary checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-C-001 | Import state remains workflow-local | Contract | Import workflow files available | Review include graph | Import state does not leak into unrelated UI source modules |

## Test File Guidance

Recommended file layout:

```text
ui/
  tests/
    support/
      FakeStorageManager.h
      WorkspaceTestData.h
    unit/
      TestAnalysisPayloadMapper.cpp
      TestAnalysisWorkflow.cpp
      TestExportWorkflow.cpp
      TestImportSuggestionService.cpp
      TestImportWorkflow.cpp
      TestNavigationState.cpp
      TestSelectionState.cpp
      TestSessionModels.cpp
      TestSessionMutationState.cpp
      TestWorkspaceFacade.cpp
      TestWorkspaceRowProjector.cpp
      TestWorkspaceSessionSelection.cpp
      TestWorkspaceSessionState.cpp
    interaction/
      TestImportState.cpp
```

## Implementation Order

1. Workspace facade and session projection.
2. Session selection, navigation, and mutation helpers.
3. Row projection and mapper smoke checks.
4. Analysis workflow filter composition and preview checks.
5. Export and import workflow smoke checks.
6. Import interaction smoke coverage.

## Definition of Done

The `ui/src` test suite is complete when:

- the workspace facade boundary is covered by behavior tests
- session models, selection, navigation, and mutation state are covered by
  stable family-based tests
- row projection helpers are covered for every workspace family
- mapper smoke tests remain buildable and usable
- export and import workflow smoke tests remain buildable and usable
- analysis filter composition and preview behavior stay stable across default and partial-selection cases
- the import workflow boundary remains smoke-tested
- the document tree matches the actual `ui/tests` tree
