# UI Source Testing Matrix

## Purpose

This document captures the behavioral test matrix for the UI source layer that
sits between QML and core. It is a companion document for the UI refactor plan
in `docs/design/tmp/ui_architecture_target.md` and keeps the source-layer test
contract explicit while the implementation is being reorganized.

The focus is parity, determinism, and contract correctness across the UI model
stack. The matrix below is intentionally not rewritten in content; only the
documentation framing is aligned with the current target architecture.

## Scope
Included in this matrix:
- `ui/models/ActorList`
- `ui/models/PropertyList`
- `ui/models/ContractList`
- `ui/controllers/ActorController`
- `ui/controllers/PropertyController`
- `ui/controllers/ContractController`
- `ui/qml/FossRedder/Views/Actor/*`
- `ui/qml/FossRedder/Views/Property/*`
- `ui/qml/FossRedder/Views/Contract/*`

Out of scope:
- Core domain behavior
- Persistence schema behavior
- Import workflow controllers and draft screens
- QML view layout and styling

In scope for the views:
- sidebar selection behavior
- prev/next navigation inside the form
- create/edit mode switching
- add/remove alias interactions
- selection panels for foreign keys in Contract
- save/delete command dispatch through the controller

## Relationship To The UI Refactor

The matrix remains valid during the UI restructure because it describes
observable behavior, not folder names.

The current target architecture keeps the same broad responsibilities:

- `ui/presentation` remains the QML-facing controller and facade layer
- `ui/viewmodels` remains the Qt model and draft model layer
- `ui/adapters` remains the translation layer between ports and UI types
- `ui/state` remains UI-only state and selection storage

The test names and coverage here should stay stable even if the source files are
moved or renamed during the refactor.

## Architectural Notes

### List models
The three list models are thin QML-facing adapters over `IndexedListModel<T>`.
They provide:
- typed item storage
- stable row lookup by id
- role names for QML
- entity-specific role serialization
- small helper methods for QML convenience, such as `add...()` and `removeAt()`

The generic base already implements the actual list mechanics:
- `setItems()` resets the model content
- `appendItem()` inserts a row and updates the internal id index
- `removeItemAt()` removes a row and rebuilds the id index
- `findIndexedRow()` resolves an entity id to a row number

The concrete list classes remain necessary because they define the domain-specific public API and payload shape for their respective entity type.

### Controllers
The three controllers are the QML-facing command surface for the respective entity type.
They provide:
- read access to a single entity
- read access to the collection
- create/update/delete commands
- save convenience methods that switch between create and update paths

They are intentionally thin and should only translate QML values into core-compatible values.

### Views
The Actor, Property, and Contract views are thin composition shells over their form and sidebar components.
Their behavior is still important because they wire the user-facing actions:
- list selection in the sidebar
- create/update/delete buttons in the bottom bar
- prev/next navigation between entities
- alias add/remove controls
- Contract-specific foreign key selection for actors and properties

These behaviors should be tested at the QML interaction layer with a fake or real `appContext`.

## Testing Strategy

The recommended coverage matrix is split into four layers:
1. model unit tests for the generic list mechanics
2. model unit tests for the concrete list adapters
3. controller tests for facade command forwarding
4. interaction tests that combine controller behavior with a real or fake application facade
5. QML interaction tests that exercise the Actor, Property, and Contract views with a fake app context

The core idea is that list model tests should verify list semantics and row mapping, while controller tests should verify command translation and parity with the facade.

For the QML views, prefer a fake app context or fake controller facade when the test only needs to observe command calls and selected-id updates. Use the real app context only when the interaction depends on shared selection state or model refresh behavior.

Navigation between items in the sidebar, create-mode toggles, and the bottom-bar buttons are QML-layer behaviors. They belong in QML tests because they verify wiring between controls, session selection, and controller calls rather than list-model or controller translation alone.

The controller and list-model tables should stay focused on data translation and command forwarding. They should not be stretched to cover visible navigation flows unless the underlying logic is purely source-layer state handling.

## Test Matrix

### Legend
- **Scope**: functional behavior under test
- **Layer**: unit, interaction, or UI-adjacent behavior
- **Setup**: minimal preconditions
- **Action**: the event, API call, or mutation executed
- **Expected**: the observable outcome

---

## 1. Shared list-model behavior

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| LST-001 | Empty model reports zero rows | Unit | Fresh list model | Query `rowCount()` | Returns `0` |
| LST-002 | Set items replaces content | Unit | Model with existing rows | Call `setItems()` with a new collection | Previous rows are discarded and the new items become visible |
| LST-003 | Append item grows list | Unit | Empty model | Call `appendItem()` | Row count increases by one and the item is reachable by row |
| LST-004 | Append item updates id index | Unit | Model with at least one item | Append entity with id | `findIndexedRow(id)` returns the inserted row |
| LST-005 | Remove item shrinks list | Unit | Model with at least two items | Call `removeItemAt(validRow)` | Row count decreases by one |
| LST-006 | Remove invalid row is ignored | Unit | Model with one item | Call `removeItemAt(-1)` or out-of-range row | No change to the model |
| LST-007 | Find row by missing id returns none | Unit | Model with items | Query `findIndexedRow("missing")` | Returns `-1` |
| LST-008 | Null items are handled safely | Unit | Model receives a collection containing null entries | Call `setItems()` | Model remains stable and only valid items contribute to lookup |

---

## 2. ActorList

### 2.1 Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-L-001 | Role names are stable | Unit | ActorList instantiated | Query `roleNames()` | Roles contain `id` and `name` |
| ACT-L-002 | Data returns actor id | Unit | Row points to valid actor | Query `data()` with `IdRole` | Returns actor id as `QString` |
| ACT-L-003 | Data returns actor name | Unit | Row points to valid actor | Query `data()` with `NameRole` | Returns actor name as `QString` |
| ACT-L-004 | Data ignores unknown roles | Unit | Row points to valid actor | Query `data()` with unknown role | Returns invalid/empty value |
| ACT-L-005 | setActors replaces all rows | Unit | ActorList with previous items | Call `setActors(newItems)` | List content matches new collection |
| ACT-L-006 | findRowById resolves current actor | Unit | ActorList contains actor with known id | Call `findRowById(id)` | Returns the correct row |
| ACT-L-007 | addActor creates typed item | Unit | ActorList ready for mutation | Call `addActor(name)` | A new actor is appended with matching name |
| ACT-L-008 | removeAt removes selected row | Unit | ActorList with multiple actors | Call `removeAt(row)` | The row is removed |

### 2.2 Unit tests

| ID | Test case | Focus | Notes |
|---|---|---|---|
| ACT-L-U-001 | `roleNames_areStable` | API contract | Protects QML bindings from role name drift |
| ACT-L-U-002 | `data_returnsNameAndId` | payload mapping | Verifies the list model serializes the correct fields |
| ACT-L-U-003 | `findRowById_usesInternalIdIndex` | indexing | Verifies the `IndexedListModel` integration |
| ACT-L-U-004 | `addActor_appendsNewItem` | mutation | Ensures the list helper uses the generic append path |
| ACT-L-U-005 | `removeAt_removesRow` | mutation | Ensures the list helper delegates to the generic remove path |

### 2.3 Interaction tests

| ID | Scenario | Setup | Action | Expected |
|---|---|---|---|---|
| ACT-L-I-001 | Rebuild from facade state | Facade provides actor collection | Call `setActors()` from state sync | Model mirrors state deterministically |
| ACT-L-I-002 | Selection row survives updates | Actor list updated after edit | Refresh model with same id present | `findRowById()` still resolves the actor |
| ACT-L-I-003 | Deleted actor disappears | Actor removed in core | Refresh model from facade | Row count decreases and id lookup fails |

---

## 3. PropertyList

### 3.1 Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-L-001 | Role names are stable | Unit | PropertyList instantiated | Query `roleNames()` | Roles contain `id` and `name` |
| PROP-L-002 | Data returns property id | Unit | Row points to valid property | Query `data()` with `IdRole` | Returns property id as `QString` |
| PROP-L-003 | Data returns property name | Unit | Row points to valid property | Query `data()` with `NameRole` | Returns property name as `QString` |
| PROP-L-004 | Data ignores unknown roles | Unit | Row points to valid property | Query `data()` with unknown role | Returns invalid/empty value |
| PROP-L-005 | setProperties replaces all rows | Unit | PropertyList with previous items | Call `setProperties(newItems)` | List content matches new collection |
| PROP-L-006 | findRowById resolves current property | Unit | PropertyList contains property with known id | Call `findRowById(id)` | Returns the correct row |
| PROP-L-007 | addProperty creates typed item | Unit | PropertyList ready for mutation | Call `addProperty(name)` | A new property is appended with matching name |
| PROP-L-008 | removeAt removes selected row | Unit | PropertyList with multiple properties | Call `removeAt(row)` | The row is removed |

### 3.2 Unit tests

| ID | Test case | Focus | Notes |
|---|---|---|---|
| PROP-L-U-001 | `roleNames_areStable` | API contract | Protects QML bindings from role name drift |
| PROP-L-U-002 | `data_returnsNameAndId` | payload mapping | Verifies the list model serializes the correct fields |
| PROP-L-U-003 | `findRowById_usesInternalIdIndex` | indexing | Verifies the `IndexedListModel` integration |
| PROP-L-U-004 | `addProperty_appendsNewItem` | mutation | Ensures the list helper uses the generic append path |
| PROP-L-U-005 | `removeAt_removesRow` | mutation | Ensures the list helper delegates to the generic remove path |

### 3.3 Interaction tests

| ID | Scenario | Setup | Action | Expected |
|---|---|---|---|---|
| PROP-L-I-001 | Rebuild from facade state | Facade provides property collection | Call `setProperties()` from state sync | Model mirrors state deterministically |
| PROP-L-I-002 | Selection row survives updates | Property list updated after edit | Refresh model with same id present | `findRowById()` still resolves the property |
| PROP-L-I-003 | Deleted property disappears | Property removed in core | Refresh model from facade | Row count decreases and id lookup fails |

---

## 4. ContractList

### 4.1 Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-L-001 | Role names are stable | Unit | ContractList instantiated | Query `roleNames()` | Roles contain `id`, `name`, `type`, `actorIds`, `propertyIds` |
| CON-L-002 | Data returns contract id | Unit | Row points to valid contract | Query `data()` with `IdRole` | Returns contract id as `QString` |
| CON-L-003 | Data returns contract name | Unit | Row points to valid contract | Query `data()` with `NameRole` | Returns contract name as `QString` |
| CON-L-004 | Data returns contract type | Unit | Row points to valid contract | Query `data()` with `TypeRole` | Returns contract type as `QString` |
| CON-L-005 | Data returns actor ids | Unit | Row points to valid contract | Query `data()` with `ActorIdsRole` | Returns actor id list |
| CON-L-006 | Data returns property ids | Unit | Row points to valid contract | Query `data()` with `PropertyIdsRole` | Returns property id list |
| CON-L-007 | setContracts replaces all rows | Unit | ContractList with previous items | Call `setContracts(newItems)` | List content matches new collection |
| CON-L-008 | findRowById resolves current contract | Unit | ContractList contains contract with known id | Call `findRowById(id)` | Returns the correct row |
| CON-L-009 | addContract creates typed item | Unit | ContractList ready for mutation | Call `addContract(name, type)` | A new contract is appended with matching name and type |
| CON-L-010 | removeAt removes selected row | Unit | ContractList with multiple contracts | Call `removeAt(row)` | The row is removed |
| CON-L-011 | get returns stable payload | Unit | ContractList with one item | Call `get(index)` | Payload contains id, name, type, actor ids, property ids |

### 4.2 Unit tests

| ID | Test case | Focus | Notes |
|---|---|---|---|
| CON-L-U-001 | `roleNames_areStable` | API contract | Protects QML bindings from role name drift |
| CON-L-U-002 | `data_returnsAllDefinedRoles` | payload mapping | Verifies the list model serializes the correct fields |
| CON-L-U-003 | `get_returnsExpectedMap` | serialization | Ensures the get helper matches the data contract |
| CON-L-U-004 | `findRowById_usesInternalIdIndex` | indexing | Verifies the `IndexedListModel` integration |
| CON-L-U-005 | `addContract_appendsNewItem` | mutation | Ensures the list helper uses the generic append path |
| CON-L-U-006 | `removeAt_removesRow` | mutation | Ensures the list helper delegates to the generic remove path |

### 4.3 Interaction tests

| ID | Scenario | Setup | Action | Expected |
|---|---|---|---|---|
| CON-L-I-001 | Rebuild from facade state | Facade provides contract collection | Call `setContracts()` from state sync | Model mirrors state deterministically |
| CON-L-I-002 | Selection row survives updates | Contract list updated after edit | Refresh model with same id present | `findRowById()` still resolves the contract |
| CON-L-I-003 | Deleted contract disappears | Contract removed in core | Refresh model from facade | Row count decreases and id lookup fails |

---

## 5. ActorController

### 5.1 Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-C-001 | Single actor lookup | Unit | Facade contains actor | Call `actor(id)` | Returns serialized payload for that actor |
| ACT-C-002 | Collection lookup | Unit | Facade contains actors | Call `actors()` | Returns all actors as QVariantList |
| ACT-C-003 | Create actor forwards aliases | Unit | Name and alias list entered | Call `addActor(name, aliases)` | Facade receives typed alias payload and returns id |
| ACT-C-004 | Update actor forwards aliases | Unit | Existing actor id present | Call `updateActor(id, name, aliases)` | Facade receives typed alias payload and updates actor |
| ACT-C-005 | Save actor create path | Unit | Empty id | Call `saveActor("", name, aliases)` | Delegates to add path |
| ACT-C-006 | Save actor update path | Unit | Non-empty id | Call `saveActor(id, name, aliases)` | Delegates to update path and returns original id |
| ACT-C-007 | Delete actor forwards id | Unit | Existing actor id present | Call `deleteActor(id)` | Facade delete method is invoked |
| ACT-C-008 | Null facade safety | Unit | Controller without facade | Call read or write API | Guard prevents dereference and returns safe default |

### 5.2 Unit test ideas

| ID | Test idea | Focus | Notes |
|---|---|---|---|
| ACT-C-U-001 | `actor_returnsPayload` | read path | Protects single-entity serialization |
| ACT-C-U-002 | `actors_returnsList` | collection path | Protects collection serialization |
| ACT-C-U-003 | `addActor_forwardsTypedAliases` | mutation translation | Verifies QStringList becomes typed alias vector |
| ACT-C-U-004 | `updateActor_forwardsTypedAliases` | mutation translation | Verifies update path parity |
| ACT-C-U-005 | `saveActor_dispatchesByIdPresence` | orchestration | Protects create/update branching |
| ACT-C-U-006 | `deleteActor_invokesFacade` | mutation translation | Ensures delete command is forwarded |

Recommended mock boundary for controller tests:
- fake `appContext`
- stub actor controller or fake facade
- real list rows only when the test needs row-to-id mapping

### 5.3 Interaction test ideas

| ID | Scenario | Setup | Action | Expected |
|---|---|---|---|---|
| ACT-C-I-001 | Actor created in core and refreshed in UI | Fake facade backed by app state | Call `addActor()` | UI receives new id and can query the actor |
| ACT-C-I-002 | Actor update preserves selection | Existing actor selected | Call `updateActor()` and refresh state | Selection continues to resolve the same actor id |
| ACT-C-I-003 | Actor deletion clears lookup | Actor deleted in core | Query controller by deleted id | Lookup returns an empty payload |

---

## 6. PropertyController

### 6.1 Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-C-001 | Single property lookup | Unit | Facade contains property | Call `property(id)` | Returns serialized payload for that property |
| PROP-C-002 | Collection lookup | Unit | Facade contains properties | Call `properties()` | Returns all properties as QVariantList |
| PROP-C-003 | Create property forwards aliases | Unit | Name and alias list entered | Call `addProperty(name, aliases)` | Facade receives typed alias payload and returns id |
| PROP-C-004 | Update property forwards aliases | Unit | Existing property id present | Call `updateProperty(id, name, aliases)` | Facade receives typed alias payload and updates property |
| PROP-C-005 | Save property create path | Unit | Empty id | Call `saveProperty("", name, aliases)` | Delegates to add path |
| PROP-C-006 | Save property update path | Unit | Non-empty id | Call `saveProperty(id, name, aliases)` | Delegates to update path and returns original id |
| PROP-C-007 | Delete property forwards id | Unit | Existing property id present | Call `deleteProperty(id)` | Facade delete method is invoked |
| PROP-C-008 | Null facade safety | Unit | Controller without facade | Call read or write API | Guard prevents dereference and returns safe default |

### 6.2 Unit test ideas

| ID | Test idea | Focus | Notes |
|---|---|---|---|
| PROP-C-U-001 | `property_returnsPayload` | read path | Protects single-entity serialization |
| PROP-C-U-002 | `properties_returnsList` | collection path | Protects collection serialization |
| PROP-C-U-003 | `addProperty_forwardsTypedAliases` | mutation translation | Verifies QStringList becomes typed alias vector |
| PROP-C-U-004 | `updateProperty_forwardsTypedAliases` | mutation translation | Verifies update path parity |
| PROP-C-U-005 | `saveProperty_dispatchesByIdPresence` | orchestration | Protects create/update branching |
| PROP-C-U-006 | `deleteProperty_invokesFacade` | mutation translation | Ensures delete command is forwarded |

Recommended mock boundary for controller tests:
- fake `appContext`
- stub property controller or fake facade
- real list rows only when the test needs row-to-id mapping

### 6.3 Interaction test ideas

| ID | Scenario | Setup | Action | Expected |
|---|---|---|---|---|
| PROP-C-I-001 | Property created in core and refreshed in UI | Fake facade backed by app state | Call `addProperty()` | UI receives new id and can query the property |
| PROP-C-I-002 | Property update preserves selection | Existing property selected | Call `updateProperty()` and refresh state | Selection continues to resolve the same property id |
| PROP-C-I-003 | Property deletion clears lookup | Property deleted in core | Query controller by deleted id | Lookup returns an empty payload |

---

## 7. ContractController

### 7.1 Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-C-001 | Single contract lookup | Unit | Facade contains contract | Call `contract(id)` | Returns serialized payload for that contract |
| CON-C-002 | Collection lookup | Unit | Facade contains contracts | Call `contracts()` | Returns all contracts as QVariantList |
| CON-C-003 | Create contract forwards actor and property ids | Unit | Name, type, actor ids, property ids and alias list entered | Call `addContract(...)` | Facade receives typed alias payload plus ids and returns id |
| CON-C-004 | Update contract forwards actor and property ids | Unit | Existing contract id present | Call `updateContract(...)` | Facade receives typed alias payload plus ids and updates contract |
| CON-C-005 | Save contract create path | Unit | Empty id | Call `saveContract("", ...)` | Delegates to add path |
| CON-C-006 | Save contract update path | Unit | Non-empty id | Call `saveContract(id, ...)` | Delegates to update path and returns original id |
| CON-C-007 | Delete contract forwards id | Unit | Existing contract id present | Call `deleteContract(id)` | Facade delete method is invoked |
| CON-C-008 | Null facade safety | Unit | Controller without facade | Call read or write API | Guard prevents dereference and returns safe default |

### 7.2 Unit test ideas

| ID | Test idea | Focus | Notes |
|---|---|---|---|
| CON-C-U-001 | `contract_returnsPayload` | read path | Protects single-entity serialization |
| CON-C-U-002 | `contracts_returnsList` | collection path | Protects collection serialization |
| CON-C-U-003 | `addContract_forwardsTypedAliasesAndIds` | mutation translation | Verifies QStringList becomes typed alias vector and id lists are preserved |
| CON-C-U-004 | `updateContract_forwardsTypedAliasesAndIds` | mutation translation | Verifies update path parity |
| CON-C-U-005 | `saveContract_dispatchesByIdPresence` | orchestration | Protects create/update branching |
| CON-C-U-006 | `deleteContract_invokesFacade` | mutation translation | Ensures delete command is forwarded |

Recommended mock boundary for controller tests:
- fake `appContext`
- stub contract controller or fake facade
- real actor/property rows when the test needs FK validation or selection payloads

### 7.3 Interaction test ideas

| ID | Scenario | Setup | Action | Expected |
|---|---|---|---|---|
| CON-C-I-001 | Contract created in core and refreshed in UI | Fake facade backed by app state | Call `addContract()` | UI receives new id and can query the contract |
| CON-C-I-002 | Contract update preserves selection | Existing contract selected | Call `updateContract()` and refresh state | Selection continues to resolve the same contract id |
| CON-C-I-003 | Contract deletion clears lookup | Contract deleted in core | Query controller by deleted id | Lookup returns an empty payload |

---

## 8. Cross-cutting parity checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PAR-001 | List model parity | Unit | All three list models available | Compare public methods and semantics | Each list exposes the same base operations with only type-specific differences |
| PAR-002 | Controller parity | Unit | All three controllers available | Compare public methods and branching style | Each controller offers read, create, update, save, and delete patterns consistently |
| PAR-003 | Alias handling parity | Unit | Actor, property, and contract inputs available | Add/update entities with aliases | Alias conversion is typed and normalized consistently |
| PAR-004 | Delete parity | Interaction | Entities exist in core | Delete through each controller | Model and selection updates are deterministic and analogous |
| PAR-005 | Selection parity | Interaction | UI selection state active | Remove a selected entity through each flow | The selected row or id advances or clears predictably |

## 9. QML view interaction tests

The navigation and button behaviors for Actor, Property, and Contract belong in the QML test layer, because they are user-facing composition behavior rather than source-layer data translation. The existing `ui/tests/qml` suite already covers this style of testing, so the matrix below should be mirrored there rather than moved into the controller/list-model unit tables.

If a behavior is driven by `selectedActorId`, `selectedPropertyId`, `selectedContractId`, or by button clicks in the form/footer/sidebar, it should be tested in the QML layer. If a behavior only converts inputs into ids or payloads, it should stay in the controller or list-model layer.

### 9.1 ActorView
| ID | Scenario | Setup | Action | Expected |
|---|---|---|---|---|
| ACT-Q-001 | Sidebar click changes selected actor | Actor sidebar and form bound to same app context | Click a different actor row in the sidebar | `selectedActorId` changes and the form reloads the new actor |
| ACT-Q-002 | Prev button selects previous actor | Actor form opened with multiple actors | Click the previous button in the bottom bar | `selectedActorId` changes to the previous row |
| ACT-Q-003 | Next navigation selects next actor | Actor form opened with multiple actors | Trigger next navigation from the form controls | The selection moves to the next actor id |
| ACT-Q-004 | Create-mode toggle clears actor selection | Actor form in edit mode | Click the create-mode toggle button | `selectedActorId` becomes empty and the form switches to create mode |
| ACT-Q-005 | Delete advances actor selection | Current actor selected | Click delete in the bottom bar | Controller delete is called and selection advances to the next available actor |
| ACT-Q-006 | Alias add appends trimmed value | Alias text entered in the form | Click the alias add button | Alias list grows and the input is cleared |
| ACT-Q-007 | Alias remove deletes selected alias | Alias chip selected | Click the alias remove button | Alias list shrinks and the alias index updates |

### 9.2 PropertyView
| ID | Scenario | Setup | Action | Expected |
|---|---|---|---|---|
| PROP-Q-001 | Sidebar click changes selected property | Property sidebar and form bound to same app context | Click a different property row in the sidebar | `selectedPropertyId` changes and the form reloads the new property |
| PROP-Q-002 | Prev button selects previous property | Property form opened with multiple properties | Click the previous button in the bottom bar | `selectedPropertyId` changes to the previous row |
| PROP-Q-003 | Next navigation selects next property | Property form opened with multiple properties | Trigger next navigation from the form controls | The selection moves to the next property id |
| PROP-Q-004 | Create-mode toggle clears property selection | Property form in edit mode | Click the create-mode toggle button | `selectedPropertyId` becomes empty and the form switches to create mode |
| PROP-Q-005 | Delete advances property selection | Current property selected | Click delete in the bottom bar | Controller delete is called and selection advances to the next available property |
| PROP-Q-006 | Alias add appends trimmed value | Alias text entered in the form | Click the alias add button | Alias list grows and the input is cleared |
| PROP-Q-007 | Alias remove deletes selected alias | Alias chip selected | Click the alias remove button | Alias list shrinks and the alias index updates |
| PROP-Q-008 | Contract selection panel updates linked contract ids | Property form with contract panel visible | Toggle a contract in the selection panel | `selectedContractIds` updates and can be saved with the property |

### 9.3 ContractView
| ID | Scenario | Setup | Action | Expected |
|---|---|---|---|---|
| CON-Q-001 | Sidebar click changes selected contract | Contract sidebar and form bound to same app context | Click a different contract row in the sidebar | `selectedContractId` changes and the form reloads the new contract |
| CON-Q-002 | Prev button selects previous contract | Contract form opened with multiple contracts | Click the previous button in the bottom bar | `selectedContractId` changes to the previous row |
| CON-Q-003 | Next navigation selects next contract | Contract form opened with multiple contracts | Trigger next navigation from the form controls | The selection moves to the next contract id |
| CON-Q-004 | Create-mode toggle clears contract selection | Contract form in edit mode | Click the create-mode toggle button | `selectedContractId` becomes empty and the form switches to create mode |
| CON-Q-005 | Delete advances contract selection | Current contract selected | Click delete in the bottom bar | Controller delete is called and selection advances to the next available contract |
| CON-Q-006 | Alias add appends trimmed value | Alias text entered in the form | Click the alias add button | Alias list grows and the input is cleared |
| CON-Q-007 | Alias remove deletes selected alias | Alias chip selected | Click the alias remove button | Alias list shrinks and the alias index updates |
| CON-Q-008 | Contract type panel updates the selected type | Contract type editor visible | Edit the type field | `contractType` changes and is persisted on save |
| CON-Q-009 | Actor FK panel updates selected actor ids | Actor selection panel visible | Toggle actor rows in the selection panel | `selectedActorIds` updates and is passed to the controller on save |
| CON-Q-010 | Property FK panel updates selected property ids | Property selection panel visible | Toggle property rows in the selection panel | `selectedPropertyIds` updates and is passed to the controller on save |

## 10. Mocking guidance

Use mocks or fakes when the test should observe UI behavior without depending on the core application implementation.

Recommended fakes:
- fake `appContext` with test doubles for `session`, `actorController`, `propertyController`, and `contractController`
- fake session selection state for sidebar and prev/next navigation tests
- fake controller methods that record calls and return deterministic ids

Do not mock the list models when the test is specifically about row lookup or payload mapping; use the real model classes there.

For Contract tests, keep actor and property rows real or minimally stubbed so FK panel behavior stays close to production behavior.

## 11. Test documentation guidance

### 9.1 Where these tests should live
- `ui/tests/unit` for pure model/controller unit tests
- `ui/tests/interaction` for controller-plus-facade tests
- `ui/tests/qml` for QML-level behavior around forms and views

### 9.2 Recommended naming convention
Use concise, behavior-first test names:
- `ActorList_addActor_appendsRow`
- `PropertyController_saveProperty_choosesCreatePath`
- `ContractController_updateContract_forwardsAliases`

### 9.3 Practical priority order
1. Add model tests for `IndexedListModel` and the three list adapters
2. Add controller unit tests for create/update/save/delete parity
3. Add interaction tests for state roundtrips through the facade
4. Keep QML tests focused on view-specific behavior, not domain orchestration

## 12. Summary
The current UI source structure is already close to parity:
- all three list models inherit the same generic base
- all three controllers expose the same CRUD-shaped public surface
- each specialized class exists for typed behavior and QML/API clarity, not for duplicating base list mechanics

The best test strategy is therefore layered and parallel:
- unit tests for generic list behavior
- unit tests for each concrete list/model/controller
- interaction tests for the end-to-end UI source flow
- QML tests for actual user-visible view behavior
