# Persistence Testing Matrix

## Purpose

This document defines the target behavioral test matrix for the `persistence`
module.

It is written to mirror the style of the other test design docs in
`docs/design/tests`: short introduction, concrete target tree, then test
matrices with explicit behavior rather than vague ideas.

The focus is correctness, deterministic save/load behavior, relation-table
fidelity, and stable deletion-impact handling for the workspace store.

## Scope

Included in this matrix:
- `persistence/src/SqliteDb.cpp`
- `persistence/src/SqliteSchema.cpp`
- `persistence/src/SqliteTransaction.cpp`
- `persistence/src/SqliteRegistry.cpp`
- `persistence/src/Factory.cpp`
- `persistence/src/AppStateStore.cpp`
- `persistence/src/repositories/*.cpp`
- `persistence/include/persistence/*.h`

Out of scope:
- `ui` behavior tests
- OCR / PDF / OpenCV behavior
- pure domain invariants that belong in `core/domain`

## Target Test Tree

```text
persistence/
  tests/
    unit/
      TestSqliteDb.cpp
      TestSqliteSchema.cpp
      TestSqliteTransaction.cpp
      TestSqliteRegistry.cpp
      TestFactory.cpp
    repositories/
      TestActorRepository.cpp
      TestPropertyRepository.cpp
      TestContractRepository.cpp
      TestStatementRepository.cpp
      TestTransactionRepository.cpp
      TestAnalysisRepository.cpp
      TestAnnualRepository.cpp
      TestStatementDraftRepository.cpp
      TestTransactionDraftRepository.cpp
      TestImportLogRepository.cpp
      TestExportLogRepository.cpp
    state/
      TestAppStateStore.cpp
      TestWorkspaceStateManager.cpp
      TestWorkspaceStorageMapper.cpp
```

## Persistence Boundary Model

The persistence layer should be tested against the refactored core boundary:

- `core/domain/catalog/WorkspaceCatalog` for catalog ownership
- `core/application/workspace/WorkspaceSessionState` for mutable application state
- `core/ports/workspace/WorkspaceSnapshot` for read-side shape
- `core/ports/workspace/WorkspaceCommands` for write-side command shape

The persistence module should store and restore the workspace state through its
repositories and orchestration layer, but it should not become a UI projection
source.

## Testing Principles

- Test each repository family through its observable contract.
- Prefer temporary file-backed SQLite fixtures over mocks.
- Verify base rows and relation-table side effects together.
- Keep deletion-impact behavior explicit and deterministic.
- Use integration tests for round-trips and cascade behavior.
- Use unit tests for the small SQLite helpers and schema lifecycle.

## Test Matrix

### Legend

- **Scope**: what behavior is validated
- **Layer**: unit, integration, or contract
- **Setup**: minimal preconditions
- **Action**: the operation under test
- **Expected**: observable result

---

## 1. DB wrapper and schema lifecycle

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| DBS-001 | Opens a valid SQLite file | Unit | Temporary path | Construct `SqliteDb` | Connection opens and schema initialization runs |
| DBS-002 | Closes the database cleanly | Unit | Opened `SqliteDb` instance | Destroy the instance | No crash and file remains usable |
| DBS-003 | Initializes schema on first open | Integration | Fresh database file | Open `SqliteDb` | Core tables exist after construction |
| DBS-004 | Reopening an existing database is stable | Integration | Database file already initialized | Open `SqliteDb` again | Schema remains valid and the connection opens |
| DBS-005 | Foreign keys are enabled | Integration | Fresh or existing database | Query `PRAGMA foreign_keys` | Returns `1` |
| DBS-006 | `user_version` is set after migration | Integration | Fresh database | Open `SqliteDb` | Schema version is at least the current migration version |
| DBS-007 | Factory produces usable DB instances | Unit | Factory inputs | Create database through factory | Returned DB object is ready for use |

---

## 2. Transaction guard

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| TXN-001 | Starts a transaction immediately | Unit | Open database handle | Construct `SqliteTransaction` | A write transaction begins |
| TXN-002 | Commit persists writes | Integration | Open transaction and insert a row | Call `commit()` | Row remains after guard destruction |
| TXN-003 | Destruction rolls back uncommitted writes | Integration | Open transaction and insert a row | Destroy guard without commit | Row is not persisted |
| TXN-004 | Double commit is harmless | Unit | Open transaction | Call `commit()` twice | Second call does nothing |
| TXN-005 | Constructor rejects null handles | Unit | Null handle | Construct guard | Exception is thrown |

---

## 3. Registry behavior

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| REG-001 | Missing latest path returns empty | Unit | Fresh registry database | Call `getLatest()` | Returns no value |
| REG-002 | Set latest path persists value | Unit | Fresh registry database | Call `setLatest("/tmp/latest.db")` | `getLatest()` returns the same path |
| REG-003 | Latest path survives instance recreation | Integration | Write value with one instance | Destroy and reopen registry | Value is still available |
| REG-004 | Empty latest path is deterministic | Unit | Fresh registry database | Store empty path | Retrieval result is deterministic and documented |

---

## 4. Repository contract matrix

### 4.1 Actor repository

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-R-001 | Add actor stores row | Contract | Empty repository | Call `addActor(actor)` | Actor is retrievable by id |
| ACT-R-002 | Get all actors returns stored rows | Contract | Repository contains actors | Call `getActors()` | Returns all rows in stable order |
| ACT-R-003 | Update actor persists changes | Contract | Existing actor | Call `updateActor(actor)` | Updated fields are visible on reload |
| ACT-R-004 | Upsert actor inserts when missing | Contract | Missing actor id | Call `upsertActor(actor)` | Actor is inserted |
| ACT-R-005 | Remove actor deletes row | Contract | Existing actor | Call `removeActor(id)` | Actor no longer exists |
| ACT-R-006 | Clear actors removes all rows | Contract | Repository contains actors | Call `clearActors()` | Repository becomes empty |
| ACT-R-007 | Actor aliases round-trip | Integration | Actor with aliases | Save and reload | Alias list and usage data are preserved |

### 4.2 Property repository

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-R-001 | Add property stores row | Contract | Empty repository | Call `addProperty(property)` | Property is retrievable by id |
| PROP-R-002 | Get all properties returns stored rows | Contract | Repository contains properties | Call `getProperties()` | Returns all rows in stable order |
| PROP-R-003 | Update property persists changes | Contract | Existing property | Call `updateProperty(property)` | Updated fields are visible on reload |
| PROP-R-004 | Upsert property inserts when missing | Contract | Missing property id | Call `upsertProperty(property)` | Property is inserted |
| PROP-R-005 | Remove property deletes row | Contract | Existing property | Call `removeProperty(id)` | Property no longer exists |
| PROP-R-006 | Clear properties removes all rows | Contract | Repository contains properties | Call `clearProperties()` | Repository becomes empty |
| PROP-R-007 | Property aliases round-trip | Integration | Property with aliases | Save and reload | Alias list and usage data are preserved |

### 4.3 Contract repository

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-R-001 | Add contract stores row | Contract | Empty repository | Call `addContract(contract)` | Contract is retrievable by id |
| CON-R-002 | Contract actor relation round-trips | Integration | Contract with actor ids | Save and reload | `actorIds` are preserved |
| CON-R-003 | Contract property relation round-trips | Integration | Contract with property ids | Save and reload | `propertyIds` are preserved |
| CON-R-004 | Update contract refreshes relations | Integration | Existing contract with relations | Call `updateContract(contract)` | Relation tables match the latest ids |
| CON-R-005 | Contract query by actor works | Contract | Contract linked to actor | Call `getContractsForActor(actorId)` | Contract is returned |
| CON-R-006 | Contract query by property works | Contract | Contract linked to property | Call `getContractsForProperty(propertyId)` | Contract is returned |
| CON-R-007 | Clear contracts removes base and relation rows | Integration | Repository contains contracts | Call `clearContracts()` | Base rows and relation rows are removed |

### 4.4 Statement repository

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| STMT-R-001 | Add statement stores row | Contract | Empty repository | Call `addStatement(statement)` | Statement is retrievable by id |
| STMT-R-002 | Statement transaction relation round-trips | Integration | Statement with transaction ids | Save and reload | `transactionIds` are preserved |
| STMT-R-003 | Update statement persists name | Contract | Existing statement | Call `updateStatement(statement)` | Updated name is visible |
| STMT-R-004 | Clear statements removes all rows | Contract | Repository contains statements | Call `clearStatements()` | Repository becomes empty |

### 4.5 Transaction repository

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| TX-R-001 | Add transaction stores row | Contract | Empty repository | Call `addTransaction(transaction)` | Transaction is retrievable by id |
| TX-R-002 | Transaction property relation round-trips | Integration | Transaction with property ids | Save and reload | `propertyIds` are preserved |
| TX-R-003 | Update transaction persists optional foreign keys | Contract | Existing transaction | Call `updateTransaction(transaction)` | `statementId`, `actorId`, and `contractId` persist as expected |
| TX-R-004 | Assign transactions to contract updates rows | Integration | Unassigned transactions exist | Call `assignTransactionsToContract(contractId, ids)` | Transactions are linked to the contract |
| TX-R-005 | Query by contract returns linked transactions | Contract | Contract-linked transactions exist | Call `getTransactionsForContract(contractId)` | Linked transactions are returned |
| TX-R-006 | Clear transactions removes rows and relations | Contract | Repository contains transactions | Call `clearTransactions()` | Transaction rows and relation rows are removed |

### 4.6 Analysis repository

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-R-001 | Add analysis stores row | Contract | Empty repository | Call `addAnalysis(analysis)` | Analysis is retrievable by id |
| ANL-R-002 | JSON fields round-trip | Contract | Analysis with config and filter JSON | Save and reload | JSON fields are preserved verbatim |
| ANL-R-003 | Update analysis persists timestamps | Contract | Existing analysis | Call `updateAnalysis(analysis)` | `createdAt` and `updatedAt` persist as expected |
| ANL-R-004 | Clear analyses removes rows | Contract | Repository contains analyses | Call `clearAnalyses()` | Repository becomes empty |
| ANL-R-005 | Analysis removal requires annual relation cleanup | Integration | Annual references existing analysis ids | Save updated annual snapshot without that analysis | Annual remains but its analysisIds no longer contain the removed id |

### 4.7 Annual repository

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-R-001 | Add annual stores row | Contract | Empty repository | Call `addAnnual(annual)` | Annual is retrievable by id |
| ANN-R-002 | Annual analysis ids round-trip | Integration | Annual with analysis ids | Save and reload | `analysisIds` are preserved in order |
| ANN-R-003 | Update annual persists year | Contract | Existing annual | Call `updateAnnual(annual)` | Year is updated |
| ANN-R-004 | Clear annuals removes rows | Contract | Repository contains annuals | Call `clearAnnuals()` | Repository becomes empty |
| ANN-R-005 | Missing analysis id is removed from annual relation table | Integration | Annual persists relation to a removed analysis | Save state with analysis removed | `annual_analyses` no longer contains the removed analysis id |

### 4.8 Statement draft repository

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SD-R-001 | Add statement draft stores row | Contract | Empty repository | Call `addStatementDraft(draft)` | Draft is retrievable by id |
| SD-R-002 | Draft transaction ids round-trip | Integration | Draft with transaction draft ids | Save and reload | `transactionIds` are preserved in order |
| SD-R-003 | Update draft persists name | Contract | Existing draft | Call `updateStatementDraft(draft)` | Updated name is visible |
| SD-R-004 | Clear drafts removes all rows | Contract | Repository contains drafts | Call `clearStatementDrafts()` | Repository becomes empty |

### 4.9 Transaction draft repository

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| TD-R-001 | Add transaction draft stores row | Contract | Empty repository | Call `addTransactionDraft(draft)` | Draft is retrievable by id |
| TD-R-002 | Property ids round-trip | Integration | Draft with property ids | Save and reload | `propertyIds` are preserved in order |
| TD-R-003 | Optional foreign keys persist | Contract | Draft with statementDraftId, actorId, contractId | Save and reload | Optional fields are persisted consistently |
| TD-R-004 | Clear drafts removes rows and relation rows | Integration | Repository contains transaction drafts | Call `clearTransactionDrafts()` | Base rows and property relation rows are removed |

### 4.10 Export log repository

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-R-001 | Add export log stores row | Contract | Empty repository | Call `addExportLog(log)` | Log is retrievable by id |
| EXP-R-002 | Annual ids round-trip | Integration | Export log with annual ids | Save and reload | `annualIds` are preserved in order |
| EXP-R-003 | Analysis ids round-trip | Integration | Export log with analysis ids | Save and reload | `analysisIds` are preserved in order |
| EXP-R-004 | Update export log refreshes payload | Contract | Existing export log | Call `updateExportLog(log)` | Fields are updated and relation rows are rewritten |
| EXP-R-005 | Clear export logs removes rows | Contract | Repository contains export logs | Call `clearExportLogs()` | Repository becomes empty |

### 4.11 Import log repository

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-R-001 | Add import log stores row | Contract | Empty repository | Call `addImportLog(log)` | Log is retrievable by id |
| IMP-R-002 | Statement draft ids round-trip | Integration | Import log with draft ids | Save and reload | `statementDraftIds` are preserved in order |
| IMP-R-003 | Update import log refreshes status | Contract | Existing import log | Call `updateImportLog(log)` | Updated status and message are visible |
| IMP-R-004 | Clear import logs removes rows | Contract | Repository contains import logs | Call `clearImportLogs()` | Repository becomes empty |

### Repository contract rules

- Use one repository family per fixture.
- Prefer temporary file-backed SQLite instead of mocks.
- Always verify both the base row and relation-table side effects.
- Explicitly test ordering for relation vectors.
- Include at least one round-trip test per repository family.

---

## 5. Workspace state orchestration

### 5.1 Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| AST-001 | Save empty session state to fresh DB | Integration | Temporary database | Call `save(emptyState)` | Save succeeds and database remains valid |
| AST-002 | Load from empty DB returns empty session state | Integration | Fresh database | Call `load()` | All collections are empty |
| AST-003 | Full session round-trips | Integration | State with catalog, drafts, and logs | Call `save(state)` then `load()` | Loaded state matches the saved state |
| AST-004 | Deleted actor is reported in deletion impact | Integration | Persisted state with actor, then remove actor from new snapshot | Call `save(newState)` | Removed actor id is reported |
| AST-005 | Deleted statement is reported in deletion impact | Integration | Persisted state with statement, then remove statement from new snapshot | Call `save(newState)` | Removed statement id is reported |
| AST-006 | Cascaded transaction deletion is suppressed when statement is removed | Integration | Persisted statement and transaction | Remove only the statement in new snapshot | Transaction is removed by cascade and not double-counted |
| AST-007 | Removed standalone transaction is reported | Integration | Persisted transaction not tied to removed statement | Remove transaction from snapshot | Removed transaction id is reported |
| AST-008 | Analysis removal updates annual references | Integration | Annuals reference an analysis | Remove analysis from snapshot | Annuals remain and no longer reference the deleted analysis |
| AST-009 | Contract removal keeps unrelated transactions stable | Integration | Transactions reference a contract | Remove contract from snapshot | Contract is removed and transactions survive with nullable FK behavior |
| AST-010 | Draft removal cleans nested draft rows | Integration | Statement draft contains transaction drafts | Remove statement draft from snapshot | Draft and dependent transactions are removed together |
| AST-011 | Save is transactional on failure | Integration | Inject a failing write path or schema violation | Call `save(state)` | No partial state is committed |
| AST-012 | Load after save preserves relation topology | Integration | State with relation tables populated | Save and reload | Relation ids, aliases, timestamps, and relation tables remain consistent |

### 5.2 Save-flow analysis

The current save flow is architecturally acceptable, but it should still be
tested as a critical boundary:

- deletion planning must stay deterministic
- statement deletions must not double-report transaction cascades
- analysis deletions must clean annual references
- repository failures must not leave partial state behind

### 5.3 Recommended `AppStateStore` / `WorkspaceStateManager` tests

| ID | Test case | Focus | Notes |
|---|---|---|---|
| AST-U-001 | `save_emptyState_succeeds` | baseline | Guards against regressions in the transaction wrapper |
| AST-U-002 | `save_reportsRemovedEntities` | deletion impact | Verifies deterministic deletion reporting |
| AST-U-003 | `save_preservesCascadedTransactionDeletionPolicy` | dependency rule | Protects the statement→transaction suppression logic |
| AST-U-004 | `load_afterSave_roundTripsAggregate` | parity | Verifies full aggregate round-trip |

### 5.4 Boundary improvement tests

These are the tests that should exist after a small refactor:

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| AST-B-001 | Deletion planning is deterministic | Unit | Existing ids and snapshot ids | Call a planning helper | Returned delete sets are stable and minimal |
| AST-B-002 | Dependency-aware deletion order is explicit | Unit | Statement and transaction ids present | Call plan builder | Statements are deleted before dependent transaction suppression is applied |
| AST-B-003 | Save failure aborts the transaction | Integration | Repository failure injected | Call `save(state)` | Transaction rolls back and result signals failure |
| AST-B-004 | Analysis removal updates annual relation rows | Unit | Existing annual ids and removed analysis id | Run the deletion planner | Annual relation cleanup is captured without deleting the annual |

---

## 6. Cross-cutting parity checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PAR-001 | All repository families support deterministic round-trip behavior | Contract | One sample entity per repository | Save and load each repository | The same entity shape survives round-trip |
| PAR-002 | All relation tables preserve ordering | Integration | Relation vectors with multiple ids | Save and reload | Order matches insertion order or documented ordering rule |
| PAR-003 | All clear operations remove side tables too | Integration | Repository with related rows | Call `clear...()` | Base rows and relation rows disappear together |
| PAR-004 | All repositories are tolerant of empty ids in read APIs | Contract | Empty string input | Call `getById("")` | Returns empty or none rather than throwing |
| PAR-005 | All repository constructors reject null DB handles | Unit | Null `SqliteDb` pointer | Construct repository | Exception is thrown |

---

## 7. Recommended file layout

- `persistence/tests/unit/TestSqliteDb.cpp`
- `persistence/tests/unit/TestSqliteSchema.cpp`
- `persistence/tests/unit/TestSqliteTransaction.cpp`
- `persistence/tests/unit/TestSqliteRegistry.cpp`
- `persistence/tests/unit/TestFactory.cpp`
- `persistence/tests/repositories/TestActorRepository.cpp`
- `persistence/tests/repositories/TestTransactionRepository.cpp`
- `persistence/tests/repositories/TestStatementDraftRepository.cpp`
- `persistence/tests/state/TestAppStateStore.cpp`
- `persistence/tests/state/TestWorkspaceStateManager.cpp`

## 8. Practical priorities

1. Add lifecycle tests for `SqliteDb`, `SqliteSchema`, and `SqliteTransaction`.
2. Add one contract test file per repository family.
3. Add explicit round-trip tests for relation tables.
4. Add state-store save/load tests that verify deletion impact.
5. Add failure-path tests once repositories return meaningful write status.

## 9. Cleanup target for old tests

Legacy persistence tests should be removed only after the new matrix covers the
same behavior with clearer intent and stronger assertions.
