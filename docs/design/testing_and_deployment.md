# Testing and Deployment — FOSSredder

Date: 2026-01-06
Author: Wilhelm Altemeier
Status: Draft

Summary
This document outlines the pragmatic test strategy and deployment baseline for the current C++/Qt application.

## Testing Strategy

### Test Layers

1. Unit tests (`core/tests/unit`, `ui/tests/unit`)
   - Pure business logic and deterministic controller/state transformations.
2. Interaction/integration tests (`ui/tests/interaction`)
   - Cross-component state flows (controller + session + model projections).
3. QML component tests (`ui/tests/qml`)
   - UI bindings, click handlers, and signal payload wiring.
4. End-to-end tests (next phase)
   - Critical user journeys from import to booking/finalize outcomes.

### Import + CRUD Test Matrix (Preparation)

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-001 | Single import | Interaction | One PDF queued | Start import | One run log, status `Draft`, draft view visible |
| IMP-002 | Draft return/resume | Interaction | Draft focused on transaction `k/n` | Return, click same draft log | Reopens same draft and same transaction index `k/n` |
| IMP-003 | Two imports background | Interaction | Two PDFs queued | Start, keep first draft open | Second import completes in background without forced view jump |
| IMP-004 | Draft switch via log | Interaction/QML | Two draft logs available | Click draft log A/B | Deterministic switch to selected draft |
| IMP-005 | Finalize while background run active | Interaction | First draft open, second import running | Finalize first draft | No stuck running state, logs remain stable and non-overwritten |
| IMP-006 | Discard while background run active | Interaction | First draft open, second import running | Discard first draft | No stuck running state, next draft remains accessible |
| IMP-007 | Finalized log navigation | QML/Interaction | Finalized log with statement id | Click finalized log | Booking section opens and correct statement is selected |
| IMP-008 | Booking sidebar visibility | QML | Many statements/transactions | Select statement via navigation/log click | Sidebar scrolls to selected statement and transactions are expandable |
| CRUD-ACT-001 | Actor CRUD | Interaction | Empty/seed actor set | Create, update, delete actor | List/selection and repository state stay consistent |
| CRUD-PROP-001 | Property CRUD | Interaction | Empty/seed property set | Create, update, delete property | List/selection and references stay consistent |
| CRUD-CON-001 | Contract CRUD | Interaction | Seed actors/properties | Create/update/delete contract relations | Relation ids persist and UI reflects changes |
| CRUD-STM-001 | Statement CRUD | Interaction | Seed statements with transactions | Create/select/delete statement | Selection consistency and cascade behavior are correct |
| CRUD-TX-001 | Transaction CRUD | Interaction | Seed statement with transactions | Create/update/delete transaction | Statement linkage, status, and list rendering are correct |
| CRUD-CROSS-001 | Cross-domain workflow | E2E | Seed domain data + importable PDF | Import -> draft edit -> finalize | Resulting statement/transactions appear correctly in booking and domain views |

### Placement Guidance

- Keep this matrix in `docs/design/testing_and_deployment.md` as the central planning artifact.
- Link to concrete test case implementations from this matrix once tests are added.
- Do not create separate matrix files per single target unless the table becomes too large; split only when maintainability requires it (e.g., dedicated `docs/design/testing/import.md`, `docs/design/testing/crud.md`).

CI
TODO: CI steps (restore vcpkg cache, build, run tests, produce artifacts). Reference detailed CI config in `docs/developer/`.

Deployment
TODO: Packaging options (installer, portable zip), signing policy, release channels and artifact expectations.