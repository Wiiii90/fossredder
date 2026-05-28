# UI QML Testing Matrix

## Purpose

This document defines the stable QML-level test matrix for the refactored UI.
It covers user-visible behavior at the QML boundary and stays aligned with the
root UI target architecture in `ui_architecture_target.md`.

The matrices below document the implemented QML surface and should remain
stable across internal source refactors as long as observable behavior stays
the same.

The suite is intentionally family-based: when a visible rule applies to every
entity editor, every sidebar, or every common control, the test should express
that reusable behavior instead of a one-off scene.

## Scope

Included in this matrix:
- `ui/tests/qml/*`
- QML components under `ui/qml/FossRedder/*`

Out of scope:
- C++ source-layer behavior
- `core` domain and application behavior
- persistence and infrastructure adapters
- visual styling changes that do not affect observable behavior

## Target Test Tree

```text
ui/
  tests/
    qml/
      qmltests.cpp
      Lookup.js
      common/
        tst_Button.qml
        tst_BottomBar.qml
        tst_CheckBox.qml
        tst_DropdownMenu.qml
        tst_Panel.qml
        tst_ProgressBar.qml
        tst_RunLogList.qml
        tst_Toolbar.qml
        tst_TextField.qml
      actor/
        tst_ActorView.qml
        tst_ActorSidebar.qml
        tst_ActorForm.qml
        tst_ActorBottomBar.qml
        tst_ActorContractPanel.qml
      property/
        tst_PropertyView.qml
        tst_PropertyForm.qml
        tst_PropertyBottomBar.qml
        tst_PropertySidebar.qml
        tst_PropertyContractPanel.qml
      contract/
        tst_ContractView.qml
        tst_ContractForm.qml
        tst_ContractSidebar.qml
        tst_ContractBottomBar.qml
        tst_ContractActorsPanel.qml
        tst_ContractPropertiesPanel.qml
        tst_ContractTypePanel.qml
        tst_ContractAllocatablePanel.qml
      annual/
        tst_AnnualView.qml
        tst_AnnualForm.qml
        tst_AnnualTransactionsPanel.qml
      booking/
        tst_BookingView.qml
        tst_BookingStatementView.qml
        tst_BookingStatementForm.qml
        tst_BookingSidebar.qml
        tst_BookingBottomBar.qml
        tst_BookingTransactionView.qml
        tst_BookingTransactionForm.qml
        tst_BookingTransactionActorPanel.qml
        tst_BookingTransactionContractPanel.qml
        tst_BookingTransactionPropertyPanel.qml
        tst_BookingTransactionAllocatablePanel.qml
      analysis/
        tst_AnalysisView.qml
        tst_AnalysisTransactionsPanel.qml
        tst_AnalysisTableView.qml
        tst_AnalysisPlotView.qml
        tst_AnalysisForm.qml
      import/
        tst_ImportSidebar.qml
        tst_ImportView.qml
        tst_ImportForm.qml
        tst_StatementDraftView.qml
      export/
        tst_ExportSidebar.qml
        tst_ExportForm.qml
        tst_ExportPanel.qml
        tst_ExportProgressBar.qml
        tst_ExportView.qml
      settings/
        tst_SettingsExport.qml
        tst_SettingsImport.qml
        tst_SettingsMiscellaneous.qml
        tst_SettingsGeneral.qml
        tst_SettingsView.qml
```

## Testing Principles

- Test the observable QML behavior, not the C++ implementation details behind
  it.
- Keep test ids stable so the matrix remains useful when the UI internals move.
- Prefer matrix-prefixed QML test function names so the code stays easy to cross-reference with the table rows.
- Use the smallest realistic QML harness or fake app context that can express
  the interaction.
- Prefer boundary wiring tests over layout pixel tests unless layout behavior is
  the point of the test.
- Keep QML tests separate from source-layer tests so each suite owns a single
  responsibility.
- Prefer repeated behavior families over bespoke one-screen-only assertions
  unless the screen is truly a unique edge case.
- For list-backed views, include a revision-driven refresh assertion so derived
  models rebind when the workspace data changes.

## Import

### ImportView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-V-001 | Default import selection | QML/Interaction | No selected file, default path configured | Open ImportView | Import controller receives default path when no file is selected and import is idle |
| IMP-V-002 | Import start | QML/Interaction | Selected file or queued files present | Click Start | `startStatementImport()` is called |
| IMP-V-003 | Import clear | QML/Interaction | Import controller idle with current selection | Click Clear | `resetStatus()` is called |
| IMP-V-004 | Import cancel | QML/Interaction | Import controller running | Click Cancel | `cancelImport()` is called |
| IMP-V-005 | Import cancel all | QML/Interaction | Import controller running with queued imports | Click Cancel all | `cancelAllImports()` is called |
| IMP-V-006 | Draft page switch | QML/Interaction | Import controller has draft | Switch controller state to draft | Draft page becomes active |
| IMP-V-007 | Import pause/resume | QML/Interaction | Import controller running | Click Pause | `togglePause()` is called and the action label reflects Resume while paused |
| IMP-V-008 | Running action order | QML/Layout | Import controller running with queued imports | Inspect bottom-bar actions | Cancel and Cancel all stay grouped on the left, while Pause/Resume sits before the outer draft-next control |
| IMP-V-009 | Import page draft navigation | QML/Interaction/Layout | Import page has persisted drafts and no active draft view | Click outer draft navigation buttons | Draft navigation buttons sit at the far left/right bottom-bar edges and call previous/next draft navigation |
| IMP-V-010 | Running import draft navigation | QML/Layout | Import controller running while drafts exist | Inspect bottom-bar actions | Draft previous/next controls remain available at the far left/right edges around Cancel/Pause actions |
| IMP-V-011 | Import page draft navigation disabled without draft logs | QML/Interaction | Workspace may contain persisted draft data but import logs expose no draft-attached row | Inspect outer draft navigation buttons | Draft previous/next controls are disabled because there is no navigable draft in the visible draft list |
| IMP-V-012 | Draft page navigation returns to import home | QML/Interaction | Draft page is open at the outer draft-navigation edge | Click the outer next/previous draft control | The active draft clears and ImportView returns to the normal import page |

### ImportForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-F-001 | Source selection | QML | Form loaded | Inspect source selector | Supported import source labels are shown |
| IMP-F-002 | Strategy selection | QML | Form loaded | Inspect strategy selector | Supported statement strategy labels are shown |

### ImportSidebar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-S-001 | Run-log binding | QML/Interaction | Import workflow exposes persisted logs | Open the sidebar | The restored run-log rows are visible in the list |
| IMP-S-002 | Draft run navigation | QML/Interaction | Import log row has an attached draft | Click the run row | The matching persisted draft id is opened |
| IMP-S-003 | Finalized run navigation | QML/Interaction | Import log row has a finalized statement id | Click the run row | Booking section opens with the matching statement selected and transaction selection cleared |
| IMP-S-004 | Legacy draft run navigation | QML/Interaction | Import log row has Draft status but no attached-draft flag | Click the run row | The row opens the persisted draft through the log-id fallback |
| IMP-S-005 | Import log delete | QML/Interaction | Import log row is visible | Click the row delete button | The workflow receives the delete request and clears attached draft state when needed |
| IMP-S-006 | Selected draft log highlight | QML/Visual state | Import sidebar has an active draft id and a matching run-log row | Open the sidebar | The matching draft run is outlined with the theme selection color |

### StatementDraftView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-D-001 | Draft return | QML/Interaction | Draft loaded | Activate return button | Draft note is written, draft is cleared, navigation moves to booking section |
| IMP-D-002 | Draft discard | QML/Interaction | Draft loaded | Activate discard button | Persisted draft is cleared, discard note is written, navigation moves to booking section |
| IMP-D-003 | Draft finalize | QML/Interaction | Draft loaded and finalize succeeds | Activate finalize button | Draft is finalized, persisted draft is cleared, finalize note is written, navigation moves to booking section |
| IMP-D-004 | Draft finalize failure | QML/Interaction | Draft loaded and finalize fails | Activate finalize button | Failure note is written and draft remains available |
| IMP-D-005 | Transaction navigation | QML/Interaction | Draft with multiple transactions | Activate previous or next transaction button | Draft transaction index changes and snapshot persistence is requested |
| IMP-D-006 | Transaction delete | QML/Interaction | Draft with more than one transaction | Activate delete transaction button | Current transaction is removed and snapshot is persisted |
| IMP-D-007 | Transaction navigation wrap | QML/Interaction | Draft positioned at the first or last transaction | Navigate past either edge | The current transaction index wraps to the opposite edge |
| IMP-D-008 | Draft metadata and proof rendering | QML/Rendering | Current draft transaction contains metadata and proof image data | Open the statement draft view | Metadata text and proof image source are rendered from the current draft payload |
| IMP-D-009 | Transaction add | QML/Interaction | Draft with transactions loaded | Activate add transaction button | A transaction is inserted after the current transaction and the draft snapshot is persisted |
| IMP-D-010 | Amount edit survives draft refresh | QML/Interaction | Draft transaction amount field contains uncommitted user text | Edit another draft field that refreshes `draft.current` | The amount input keeps the local edit and commits only when the amount field finishes editing |
| IMP-D-011 | Amount value stays stable across transaction switches | QML/Interaction | Draft with multiple transactions and a committed amount change on the current transaction | Switch to another transaction and back | The amount field reflects each transaction value deterministically and restores the committed value when returning |
| IMP-D-012 | Property quick-create delegates to workflow | QML/Interaction | Draft loaded and property quick-create controls visible | Enter a property name and activate `+` | `createPropertyChoiceForCurrentDraft(...)` is called with the entered name |
| IMP-D-013 | Contract quick-create delegates to workflow | QML/Interaction | Draft loaded and contract quick-create controls visible | Enter contract name/type and activate `+` | `createOrSelectContractChoiceForCurrentDraft(...)` is called with entered values and selection updates through workflow helpers |
| IMP-D-014 | Actor quick-create appears immediately and is selected | QML/Interaction | Draft loaded with actor quick-create controls visible | Enter actor name and activate `+` | The new actor choice appears immediately in the actor dropdown and is selected for the current transaction |

## Export

### ExportView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-V-001 | Container mount | QML | App context and theme available | Open ExportView | `ExportPanel` and `ExportSidebar` are filled with app context and theme |
| EXP-V-002 | Start export uses selected target directory directly | QML/Interaction | Export form has a target directory and at least one export item | Click Start | `exportDataWithPayload(..., path, ...)` receives the selected directory path without appending an extra `/export` segment |

### ExportForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-F-001 | Target directory | QML | Form loaded | Edit target directory | Export settings update deterministically |
| EXP-F-002 | Archive format | QML | Form loaded | Select archive format | Selected format updates deterministically |
| EXP-F-003 | Browse action | QML | Form loaded | Activate browse button | Browse request is emitted |

### ExportPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-P-001 | Annual export binding | QML/Interaction | Annual and analysis rows available | Open panel in annual mode | Annual entries are shown and add/remove actions stay deterministic |
| EXP-P-002 | Analysis export binding | QML/Interaction | Analysis rows available | Switch to analysis mode | Analysis entries are shown and add/remove actions stay deterministic |

### ExportProgressBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-PB-001 | Progress binding | QML | Export workflow running | Inspect the progress bar | Progress value and status text follow the workflow state |

### ExportSidebar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-S-001 | Run-log binding | QML/Interaction | Export workflow exposes persisted logs | Open the sidebar | The restored run-log rows are visible in the list |

## Actor

### ActorView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-V-003 | Bottom navigation cycles through create mode | QML/Interaction | Multiple actor rows and the last/first actor selected | Click next from the last actor or previous from the first actor, then click again | Selection clears into create mode first, then continues to the opposite edge |
| ACT-V-004 | Bottom navigation starts from create mode | QML/Interaction | Multiple actor rows and no actor selected | Click next or previous | Next selects the first actor and previous selects the last actor |
| ACT-V-005 | Bottom navigation supports single row | QML/Interaction | Actor form opened with only one actor row | Click previous or next | Previous/next buttons remain enabled and navigation can select the only actor |

### ActorSidebar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-S-001 | Row binding | QML/Interaction | Sidebar loaded with actor rows | Open the sidebar | The sidebar renders the actor row labels from the workspace facade |
| ACT-S-002 | Row selection | QML/Interaction | Sidebar loaded with actor rows | Click an actor row | The selection request is forwarded through actor state |
| ACT-S-003 | Selected highlight | QML/Visual state | Sidebar loaded with a selected actor id | Open the sidebar | The matching actor row uses the theme selection highlight |
| ACT-S-004 | Sidebar scrolling | QML/Layout | Sidebar has more rows than visible height | Open the sidebar | The internal flickable becomes scrollable for the full row list |

### ActorForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-F-001 | Create actor | QML/Interaction | No selected actor | Enter name and click Create | `saveActor("", ...)` is called, form clears, selected actor id becomes returned id |
| ACT-F-002 | Alias add/delete | QML/Interaction | Create or edit mode with alias input available | Activate `+` and `-` alias buttons | Alias is trimmed, added once, removed and index updates deterministically |
| ACT-F-003 | Edit-mode alias add | QML/Interaction | Edit mode with a selected actor | Add an alias | Alias is added to actor state without leaving edit mode |
| ACT-F-004 | Alias layout space | QML/Layout | Actor form rendered with aliases | Open form | Alias panel receives stable layout space |
| ACT-F-004B | Alias chip selection | QML/Interaction | Actor form rendered with multiple aliases | Click an alias chip | Alias index updates and remove action becomes enabled |
| ACT-F-005 | Read actor state | QML/Interaction | Selected actor with name and aliases | Open form in edit mode | Fields reflect selected actor state |
| ACT-F-006 | Update actor | QML/Interaction | Selected actor with modified fields | Change name or aliases and click Update | `saveActor(current.id, ...)` is called and saved state is refreshed |
| ACT-F-007 | Update selected contracts | QML/Interaction | Selected actor with contract rows | Toggle contract checkboxes and update | Selected contract ids are saved |
| ACT-F-008 | Selection signal refresh | QML/State sync | Selected actor object emits changed | Change selected aliases externally | Form state refreshes without replacing the selection object |
| ACT-F-009 | Data revision refresh | QML/State sync | Selected actor object does not emit changed | Trigger data revision | Form state refreshes from selected actor data |
| ACT-F-010 | Contract selection | QML/Interaction | Selected actor with contract rows | Toggle contract checkboxes | Selected contract ids are updated in the form state |
| ACT-F-011 | Actor navigation | QML/Interaction | Actor rows available | Click Prev or Next | Selected actor id moves to adjacent actor |
| ACT-F-012 | Create-mode shortcut button | QML/Interaction | Edit mode with a selected actor | Click bottom-bar `+` button | Selection clears and form switches to create mode |
| ACT-F-013 | Delete actor | QML/Interaction | Selected actor with valid id | Click Delete | `deleteActor(id)` is called and selection advances deterministically |

### ActorBottomBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-BB-001 | Navigation buttons | QML/Interaction | Bottom bar loaded with actor rows and state hooks | Click Prev or Next | Navigation methods are called on the actor state |
| ACT-BB-002 | Create-mode buttons | QML/Interaction | Bottom bar loaded in create mode | Click Clear or Create | Clear and submit actions are forwarded to the actor state |
| ACT-BB-003 | Edit-mode buttons | QML/Interaction | Bottom bar loaded in edit mode with changes | Click `+`, Delete or Update | Create-mode, delete and update actions are forwarded to the actor state |

### ActorContractPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-CP-001 | Contract selection | QML/Interaction | Actor contract panel loaded with contract rows | Toggle a contract checkbox | The selected contract ids on actor state are updated |
| ACT-CP-002 | Selection rendering | QML/Visual state | Actor contract panel loaded with a selected contract id | Open the panel | The matching contract checkbox reflects the selected state |

## Property

### PropertyView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-V-003 | Bottom navigation cycles through create mode | QML/Interaction | Multiple property rows and the last/first property selected | Click next from the last property or previous from the first property, then click again | Selection clears into create mode first, then continues to the opposite edge |
| PROP-V-004 | Bottom navigation starts from create mode | QML/Interaction | Multiple property rows and no property selected | Click next or previous | Next selects the first property and previous selects the last property |
| PROP-V-005 | Bottom navigation supports single row | QML/Interaction | Property form opened with only one property row | Click previous or next | Previous/next buttons remain enabled and navigation can select the only property |
| PROP-V-006 | Contract selection enables update | QML/Interaction | Property form opened in edit mode with a changed contract selection | Toggle a contract checkbox | The update action becomes enabled once the form has changes |

### PropertySidebar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-S-001 | Row binding | QML/Interaction | Sidebar loaded with property rows | Open the sidebar | The sidebar renders the property row labels from the workspace facade |
| PROP-S-002 | Row selection | QML/Interaction | Sidebar loaded with property rows | Click a property row | The selection request is forwarded through property state |
| PROP-S-003 | Selected highlight | QML/Visual state | Sidebar loaded with a selected property id | Open the sidebar | The matching property row uses the theme selection highlight |
| PROP-S-004 | Sidebar scrolling | QML/Layout | Sidebar has more rows than visible height | Open the sidebar | The internal flickable becomes scrollable for the full row list |
| PROP-S-005 | Row refresh | QML/State sync | Sidebar is open and property rows change | Update the workspace property rows | The sidebar renders the new property row without reopening |

### PropertyBottomBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-BB-001 | Navigation buttons | QML/Interaction | Bottom bar loaded with property rows and state hooks | Click Prev or Next | Navigation methods are called on the property state |
| PROP-BB-002 | Create-mode buttons | QML/Interaction | Bottom bar loaded in create mode | Click Clear or Create | Clear and submit actions are forwarded to the property state |
| PROP-BB-003 | Edit-mode buttons | QML/Interaction | Bottom bar loaded in edit mode with changes | Click `+`, Delete or Update | Create-mode, delete and update actions are forwarded to the property state |

### PropertyForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-F-001 | Create property | QML/Interaction | No selected property | Enter name and click Create | `saveProperty("", ...)` is called, form clears, selected property id becomes returned id |
| PROP-F-002 | Read property state | QML/Interaction | Selected property with name and aliases | Open form in edit mode | Fields reflect selected property state |
| PROP-F-005 | Alias add/delete | QML/Interaction | Create or edit mode with alias input available | Activate `+` and `-` alias buttons | Alias is trimmed, added once, removed and index updates deterministically |
| PROP-F-005B | Edit-mode alias add | QML/Interaction | Edit mode with a selected property | Add an alias | Alias is added to property state without leaving edit mode |
| PROP-F-005C | Alias layout space | QML/Layout | Property form rendered with aliases | Open form | Alias panel receives stable layout space |
| PROP-F-005D | Alias chip selection | QML/Interaction | Property form rendered with multiple aliases | Click an alias chip | Alias index updates and remove action becomes enabled |
| PROP-F-006 | Update property | QML/Interaction | Selected property with modified fields | Change name or aliases and click Update | `saveProperty(current.id, ...)` is called and saved state is refreshed |
| PROP-F-007 | Contract selection | QML/Interaction | Selected property with contract rows | Toggle contract checkboxes | Selected contract ids are updated in the form state |
| PROP-F-008 | Update selected contracts | QML/Interaction | Selected property with contract rows | Toggle contract checkboxes and update | Selected contract ids are saved |
| PROP-F-010 | Selection signal refresh | QML/State sync | Selected property object emits changed | Change selected aliases externally | Form state refreshes without replacing the selection object |
| PROP-F-011 | Data revision refresh | QML/State sync | Selected property object does not emit changed | Trigger data revision | Form state refreshes from selected property data |
| PROP-F-012 | Property navigation | QML/Interaction | Property rows available | Click Prev or Next | Selected property id moves to adjacent property |
| PROP-F-013 | Create-mode shortcut button | QML/Interaction | Edit mode with a selected property | Click bottom-bar `+` button | Selection clears and form switches to create mode |
| PROP-F-014 | Delete property | QML/Interaction | Selected property with valid id | Click Delete | `deleteProperty(id)` is called and selection advances deterministically |

### PropertyContractPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-CP-001 | Contract selection | QML/Interaction | Property contract panel loaded with contract rows | Toggle a contract checkbox | The selected contract ids on property state are updated |
| PROP-CP-002 | Selection rendering | QML/Visual state | Property contract panel loaded with a selected contract id | Open the panel | The matching contract checkbox reflects the selected state |

## Contract

### ContractView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-V-003 | Bottom navigation cycles through create mode | QML/Interaction | Multiple contract rows and the last/first contract selected | Click next from the last contract or previous from the first contract, then click again | Selection clears into create mode first, then continues to the opposite edge |
| CON-V-004 | Bottom navigation starts from create mode | QML/Interaction | Multiple contract rows and no contract selected | Click next or previous | Next selects the first contract and previous selects the last contract |
| CON-V-005 | Bottom navigation supports single row | QML/Interaction | Contract form opened with only one contract row | Click previous or next | Previous/next buttons remain enabled and navigation can select the only contract |

### ContractSidebar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-S-001 | Row binding | QML/Interaction | Sidebar loaded with contract rows | Open the sidebar | The sidebar renders the contract row labels from the workspace facade |
| CON-S-002 | Row selection | QML/Interaction | Sidebar loaded with contract rows | Click a contract row | The selection request is forwarded through contract state |
| CON-S-003 | Selected highlight | QML/Visual state | Sidebar loaded with a selected contract id | Open the sidebar | The matching contract row uses the theme selection highlight |
| CON-S-004 | Sidebar scrolling | QML/Layout | Sidebar has more rows than visible height | Open the sidebar | The internal flickable becomes scrollable for the full row list |

### ContractBottomBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-BB-001 | Navigation buttons | QML/Interaction | Bottom bar loaded with contract rows and state hooks | Click Prev or Next | Navigation methods are called on the contract state |
| CON-BB-002 | Create-mode buttons | QML/Interaction | Bottom bar loaded in create mode | Click Clear or Create | Clear and submit actions are forwarded to the contract state |
| CON-BB-003 | Edit-mode buttons | QML/Interaction | Bottom bar loaded in edit mode with changes | Click `+`, Delete or Update | Create-mode, delete and update actions are forwarded to the contract state |

### ContractForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-F-001 | Create contract | QML/Interaction | No selected contract, a valid contract type, and at least one relation selected | Enter name and type, select at least one actor or property, then click Create | `saveContract("", ...)` is called, form clears, selected contract id becomes returned id |
| CON-F-002 | Alias add/delete | QML/Interaction | Create or edit mode with alias input available | Activate `+` and `-` alias buttons | Alias is trimmed, added once, removed and index updates deterministically |
| CON-F-003 | Edit-mode alias add | QML/Interaction | Edit mode with a selected contract | Add an alias | Alias is added to contract state without leaving edit mode |
| CON-F-004 | Alias layout space | QML/Layout | Contract form rendered with aliases | Open form | Alias panel receives stable layout space |
| CON-F-004B | Alias chip selection | QML/Interaction | Contract form rendered with multiple aliases | Click an alias chip | Alias index updates and remove action becomes enabled |
| CON-F-005 | Read contract state | QML/Interaction | Selected contract with type, actors, properties, aliases | Open form in edit mode | Fields and selection panels reflect selected contract state |
| CON-F-006 | Update contract | QML/Interaction | Selected contract with modified fields | Change name, type, selections or aliases and click Update | `saveContract(current.id, ...)` is called and saved state is refreshed |
| CON-F-007 | Update selected relations | QML/Interaction | Selected contract with actor and property rows | Change actor/property selections and update | Selected relation ids are saved |
| CON-F-008 | Replace primary actor | QML/Interaction | Selected contract has an assigned actor | Select a different primary actor | Previous actor assignment is replaced |
| CON-F-009 | Selection signal refresh | QML/State sync | Selected contract object emits changed | Change selected aliases externally | Form state refreshes without replacing the selection object |
| CON-F-010 | Data revision refresh | QML/State sync | Selected contract object does not emit changed | Trigger data revision | Form state refreshes from selected contract data |
| CON-F-011 | Actor selection | QML/Interaction | Actor rows available | Select the primary actor | Selected actor ids are updated in the form state |
| CON-F-012 | Property selection | QML/Interaction | Property rows available | Toggle contract property checkboxes | Selected property ids are updated in the form state |
| CON-F-013 | Relation validation | QML/Interaction | Create mode with no relation selected | Enter name and type | Create remains disabled until an actor or property is selected |
| CON-F-014 | Contract navigation | QML/Interaction | Contract rows available | Click Prev or Next | Selected contract id moves to adjacent contract |
| CON-F-015 | Create-mode shortcut button | QML/Interaction | Edit mode with a selected contract | Click bottom-bar `+` button | Selection clears and form switches to create mode |
| CON-F-016 | Delete contract | QML/Interaction | Selected contract with valid id | Click Delete | `deleteContract(id)` is called and selection advances deterministically |

### ContractActorsPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-AP-001 | Actor selection | QML/Interaction | Contract actor panel loaded with actor rows and contract state | Select an actor from the dropdown | The selected actor id is written through `selectPrimaryActor()` |
| CON-AP-002 | Existing selection rendering | QML/Visual state | Contract actor panel loaded with a selected actor id | Open the panel | The dropdown reflects the selected actor |

### ContractPropertiesPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-PP-001 | Property selection | QML/Interaction | Contract properties panel loaded with property rows | Toggle a property checkbox | The selected property ids on contract state are updated |
| CON-PP-002 | Selection rendering | QML/Visual state | Contract properties panel loaded with a selected property id | Open the panel | The matching property checkbox reflects the selected state |

### ContractTypePanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-TP-001 | Type edit | QML/Interaction | Contract type panel loaded with contract state | Edit the type field | The type is written to contract state |
| CON-TP-002 | Type rendering | QML/Visual state | Contract type panel loaded with an existing type | Open the panel | The type field reflects contract state |

### ContractAllocatablePanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-ALP-001 | Allocatable mode edit | QML/Interaction | Contract allocatable panel loaded with contract state | Select an allocatable mode | The allocatable mode is written to contract state |
| CON-ALP-002 | Allocatable mode rendering | QML/Visual state | Contract allocatable panel loaded with an existing mode | Open the panel | The dropdown reflects contract state |

## Annual

### AnnualView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-V-001 | Container mount | QML | App context and theme available | Open AnnualView | `AnnualForm` is filled with app context and theme |
| ANN-V-002 | Bottom navigation cycles through create mode | QML/Interaction | Multiple annual rows and the last/first annual selected | Click next from the last annual or previous from the first annual, then click again | Selection clears into create mode first, then continues to the opposite edge |
| ANN-V-003 | Bottom navigation starts from create mode | QML/Interaction | Multiple annual rows and no annual selected | Click next or previous | Next selects the first annual and previous selects the last annual |
| ANN-V-004 | Bottom navigation supports single row | QML/Interaction | Annual form opened with only one annual row | Click previous or next | Previous/next buttons remain enabled and navigation can select the only annual |

### AnnualForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-001 | Create annual | QML/Interaction | No selected annual, valid year entered | Enter annual name and year, click Create | `saveAnnual("", ...)` is called, selected annual id becomes returned id |
| ANN-002 | Read annual state | QML/Interaction | Selected annual with loaded analyses and transactions | Open form in edit mode | `annual(id)` resolves the live annual payload and the form fields plus derived panels reflect selected annual state |
| ANN-003 | Update annual | QML/Interaction | Selected annual with modified fields or analyses | Change name, year or analyses and click Update | `saveAnnual(current.id, ...)` is called and saved state is refreshed |
| ANN-004 | Delete annual | QML/Interaction | Selected annual with valid id | Click Delete | `deleteAnnual(id)` is called and selection advances deterministically |
| ANN-005 | Year validation | QML/Interaction | Create mode with invalid year text | Enter invalid year | Submit remains disabled |
| ANN-006 | Analysis assignment | QML/Interaction | Analysis rows available | Select analyses in panel | Assigned analysis ids are updated |
| ANN-007 | Analysis removal | QML/Interaction | Selected analyses present | Remove assigned analysis | Assigned analysis ids shrink deterministically |
| ANN-008 | Annual navigation | QML/Interaction | Annual rows available | Click Prev or Next | Selected annual id moves to adjacent annual |

### AnnualAnalysesPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-P-001 | Add annual analysis | QML | Analysis rows available | Activate add analysis button | Analysis id is added once to selection |
| ANN-P-002 | Add export format update | QML/Interaction | Assigned analysis row visible | Change export format | `updateAnalysis()` is called with normalized export format |
| ANN-P-003 | Remove analysis | QML/Interaction | Assigned analysis visible | Activate remove analysis button | Analysis id is removed from selection |
| ANN-P-004 | Assigned analysis row rendering | QML/Interaction | Selected annual has `analysisIds` and matching analysis rows | Open annual form in edit mode | Assigned analysis renders as a visible panel row (not only reflected in summary counters) |
| ANN-P-005 | Annual transaction snapshot mapping | QML/Interaction | Selected annual has assigned analyses with canonical snapshot payload arrays (`[]`) and live transaction rows | Switch to Annual Transactions workspace | Transactions are derived/rendered from `WorkspaceFacade::annualResultStatePreview` through the annual application service, including allocatable state and contract type when available |

### AnnualTransactionsPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-T-001 | Category sections render deterministically | QML/Interaction | Grouped annual transaction payload contains at least one row in `deduplicated`, `similar`, `divergent`, and `workspaceOnly` | Open annual transactions panel | All four category section toggles are visible and stay independently expandable/collapsible |

## Booking

### BookingView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-V-001 | Create-mode container mount | QML | App context and theme available with no selected statement | Open BookingView | `BookingStatementView` is filled with app context and theme in create mode |
| BKG-V-002 | Edit-mode container mount | QML | App context and theme available with BookingState in edit mode | Open BookingView and click Update | The update command is forwarded to `BookingState.updateCurrent()` |

### BookingBottomBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-BB-001 | Navigation requests | QML/Interaction | Bottom bar has statement and transaction navigation enabled | Click previous/next statement and transaction buttons | The matching navigation methods are called on `BookingState` |
| BKG-BB-002 | Create mode commands | QML/Interaction | Bottom bar is in create mode and create is enabled | Click Clear and Create | `BookingState.resetCreateState()` and `submit()` are called |
| BKG-BB-003 | Edit mode commands | QML/Interaction | Bottom bar is in edit mode and update is enabled | Click Delete and Update | `BookingState.deleteCurrentStatement()` and `updateCurrent()` are called |

### BookingStatementView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-SV-001 | Statement name state binding | QML/Interaction | BookingStatementView receives a BookingState object | Edit statement name field | `BookingState.statementName` receives the edited text |
| BKG-SV-002 | Transaction add/delete state commands | QML/Interaction | BookingStatementView receives a BookingState object with transaction commands | Click add and delete transaction buttons | `BookingState.addTransactionAfterCurrent()` and `deleteCurrentTransaction()` are called |

### BookingStatementForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-SF-001 | Statement name state binding | QML/Interaction | Form loaded with BookingState | Edit statement name field | `BookingState.statementName` is updated directly |
| BKG-SF-002 | Transaction command binding | QML/Interaction | Form loaded with transaction add/delete buttons enabled | Click add and delete transaction buttons | Transaction commands are called on `BookingState` |
| BKG-SF-003 | Transaction view composition | QML | Form loaded with BookingState | Open the form | `BookingTransactionView` is mounted inside the statement panel and receives the same state |

### BookingSidebar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-S-001 | Transaction row click | QML/Interaction | Statement rows and transaction rows available | Click a transaction row | The matching statement and transaction ids become active |
| BKG-S-002 | Statement row click | QML/Interaction | Statement rows and transaction rows available | Click a statement row | The statement remains selected and the transaction selection is cleared |
| BKG-S-003 | Statement row refresh | QML/Interaction | Statement rows and transaction rows available | Replace the statement row model | Removed transaction rows disappear from the sidebar |

### BookingTransactionView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TV-001 | Transaction view composition | QML | BookingState loaded | Open the transaction view | Transaction form, actor, contract, property, and allocatable panels are mounted with the same state |
| BKG-T-006 | Contract selection cascade | Unit | Covered by `BKG-ST-007` in the UI source matrix | Select contract in transaction | BookingState synchronizes related actor/property ids |
| BKG-T-007 | Actor incompatibility cleanup | Unit | Covered by `BKG-ST-008` in the UI source matrix | Select incompatible actor | BookingState clears incompatible contract id |
| BKG-T-008 | Property incompatibility cleanup | Unit | Covered by `BKG-ST-009` in the UI source matrix | Select incompatible property set | BookingState clears incompatible contract id |

### BookingTransactionForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TF-001 | Transaction field edit | QML/Interaction | BookingState loaded | Edit name, status, booking date, valuta, and amount fields | Matching BookingState transaction properties update |

### BookingTransactionActorPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TA-001 | Actor selection list | QML | BookingState actor display rows available | Select an actor | Selected actor index is forwarded to BookingState |

### BookingTransactionContractPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TC-001 | Contract selection list | QML | BookingState contract display rows available | Select a contract | Selected contract index is forwarded to BookingState |

### BookingTransactionPropertyPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TP-001 | Property selection list | QML | BookingState property rows available | Select a property | Property selection is forwarded to BookingState |
| BKG-TP-002 | Property selection refresh | QML | BookingState selected property ids change | Refresh the selected property ids | The checkbox checked state is derived from BookingState |

### BookingTransactionAllocatablePanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TA-002 | Allocatable mode | QML | BookingState loaded | Change allocatable mode | Mode is written to BookingState |

## Analysis

### AnalysisView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-V-001 | Container refresh | QML | App context and theme available | Show view with selection | `AnalysisForm.refreshFromSelection()` is called when view becomes visible |
| ANL-V-002 | Bottom navigation cycles through create mode | QML/Interaction | Multiple analysis rows and the last/first analysis selected | Click next from the last analysis or previous from the first analysis, then click again | Selection clears into create mode first, then continues to the opposite edge |
| ANL-V-003 | Bottom navigation starts from create mode | QML/Interaction | Multiple analysis rows and no analysis selected | Click next or previous | Next selects the first analysis and previous selects the last analysis |
| ANL-V-004 | Bottom navigation supports single row | QML/Interaction | Analysis form opened with only one analysis row | Click previous or next | Previous/next buttons remain enabled and navigation can select the only analysis |

### AnalysisForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-001 | Create analysis | QML/Interaction | No selected analysis and valid name entered | Enter name and click Create | `createAnalysis()` is called and selected analysis id becomes returned id |
| ANL-002 | Read analysis state | QML/Interaction | Selected analysis loaded | Open form in edit mode | Name, type, filters, export options and result panels reflect selected analysis |
| ANL-003 | Update analysis | QML/Interaction | Selected analysis with modified configuration | Change fields and click Update | `updateAnalysis()` is called with normalized config and filter state |
| ANL-004 | Delete analysis | QML/Interaction | Selected analysis with valid id | Click Delete | `deleteAnalysis(id)` is called and selection advances deterministically |
| ANL-005 | Navigate analysis | QML/Interaction | Analysis rows available | Click Prev or Next | Selected analysis id moves to adjacent analysis |
| ANL-006 | Reset filters | QML/Interaction | Filter edit mode with changes | Click Reset | Filters and adjustments are reset |
| ANL-007 | Toggle filter workspace | QML/Interaction | Create mode with filter workspace visible | Click toggle workspace | Filter workspace index switches deterministically |
| ANL-008 | Export format normalization | QML/Interaction | Table or plot analysis selected | Select invalid export format | Export format falls back to valid option for active type |
| ANL-009 | Property filter selection | QML/Interaction | Property rows available | Change property selection | Preview refresh is requested and selected ids update |
| ANL-010 | Contract type filter selection | QML/Interaction | Contract types available | Change contract type selection | Preview refresh is requested and selected types update |
| ANL-011 | Allocatable filter selection | QML/Interaction | Filter panel visible | Change allocatable mode | Preview refresh is requested and allocatable mode updates |
| ANL-012 | Result export state | QML/Interaction | Plot analysis selected | Modify export state | Export state JSON is normalized and persisted |
| ANL-013 | Include-calc toggle persists on update | QML/Interaction | Existing analysis in edit mode with calc adjustments enabled | Toggle `Include Calc Adjustments` and click Update | `updateAnalysis(..., includeCalcAdjustments, ...)` receives the toggled boolean and subsequent previews honor it |

### AnalysisTransactionsPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-P-001 | Transaction selection | QML | Preview transactions available | Select transactions | Selected transaction ids are forwarded |
| ANL-P-002 | Calculation application | QML/Interaction | Transactions selected and percent entered | Apply calculation | Pending adjustments JSON is produced |

### AnalysisPlotView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-PV-001 | Export state roundtrip | QML/Interaction | Plot view loaded | Change plot export state | Export state JSON is forwarded back to parent |

### AnalysisTableView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-TV-001 | Table render | QML/Interaction | Adjustment data available | Open table view | Adjustment rows render from amount map |
| ANL-TV-002 | Matrix uses readable property names from result transactions | QML/Interaction | Result transactions provide `propertyIds` and `propertyNames` | Open table view | Matrix row labels use `propertyNames` instead of raw ids where available |
| ANL-TV-003 | Matrix rebuild reacts to adjustment map changes | QML/Interaction | Table view loaded with result transactions | Update `adjustmentAmountsById` at runtime | Matrix totals are recalculated immediately to reflect adjusted values |

## Components

### EntityPicker

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ENT-001 | Actor add | QML/Interaction | Picker bound to actor model | Type name and activate add item button | `addActor()` is called and selection includes returned id |
| ENT-002 | Property add | QML/Interaction | Picker bound to property model | Type name and activate add item button | `addProperty()` is called and selection includes returned id |
| ENT-003 | Contract add | QML/Interaction | Picker bound to contract model | Type name and activate add item button | `addContract()` is called and selection includes returned id |
| ENT-004 | Duplicate selection guard | QML/Interaction | Returned id already selected | Add same item again | Selection stays deduplicated |
| ENT-005 | Add field reset | QML/Interaction | Name entered | Activate add item button | Input field clears after add |
| ENT-006 | Toggle selection | QML/Interaction | Item listed | Toggle checkbox | Id is added or removed from selected ids |

### FilePicker

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| FILE-001 | Initial folder | QML | File system controller present | Open picker | Folder defaults to application directory |
| FILE-002 | Open directory | QML/Interaction | Picker loaded | Call open(dir) | Folder and field text update to chosen directory |
| FILE-003 | Up navigation | QML/Interaction | Current folder has parent | Activate up button | Folder changes to parent path |
| FILE-004 | Directory selection | QML/Interaction | File list contains directory | Activate directory row | Folder changes to clicked directory |
| FILE-005 | File selection | QML/Interaction | File list contains file | Activate file row | Selected file changes to clicked file path |
| FILE-006 | Accept typed filename | QML/Interaction | Folder set and filename entered | Activate select button | Accepted path combines folder and filename |
| FILE-007 | Accept selected file fallback | QML/Interaction | Selected file set and filename empty | Activate select button | Accepted path uses selected file |
| FILE-008 | Cancel picker | QML/Interaction | Picker loaded | Activate cancel button | Rejected signal is emitted |

## Settings

### SettingsView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SET-V-002 | Category navigation | QML/Interaction | Settings view loaded | Click next and previous category controls | Settings category advances and returns with loaded page content |
| SET-V-003 | Update settings | QML/Interaction | Settings view model and language service available | Click Update | Settings are saved and the selected language is applied |
| SET-V-004 | Reset settings | QML/Interaction | Settings category is not the default | Click Default | Settings reset and category returns to General |
| SET-V-005 | Category navigation wraps | QML/Interaction | Settings category is at either edge | Click next from the last category or previous from the first category | Category wraps to the opposite edge |

## Common Controls and Shared Components

### Button

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-B-001 | Click signal | QML | Button loaded | Activate button | Click is emitted and visual state resets |
| CTRL-B-002 | Visual state reset helper | QML | Button pressed state active | Call clearVisualState() | `down` state is reset |
| CTRL-B-003 | Disabled state | QML | Button disabled | Activate button | Control remains non-active and visual state remains stable |

### CheckBox

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-CB-001 | Toggle state | QML | Checkbox loaded | Toggle checkbox | Checked state changes deterministically |
| CTRL-CB-002 | Layout binding | QML | Checkbox in layout | Inspect control | Checkbox fills width and aligns vertically |

### TextField

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-TF-001 | Text binding | QML | TextField loaded | Edit text | Bound text updates immediately |
| CTRL-TF-002 | Focus styling | QML | TextField focused | Inspect border | Focused border styling is applied |

### DropdownMenu

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-DD-001 | Selection change | QML | Model available | Select item | Current index and activated signal update |
| CTRL-DD-002 | Index update | QML | Model contains multiple options | Change current index | Current index updates deterministically |

### Panel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-P-001 | Default content layout | QML | Panel loaded | Inspect container | Content is placed in the panel body |
| CTRL-P-002 | Content spacing | QML | Panel loaded | Inspect layout spacing | Content spacing follows theme binding |

### ProgressBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-PB-001 | Determinate value binding | QML | Value set | Inspect control value | Progress value reflects assigned percentage |
| CTRL-PB-002 | Indeterminate binding | QML | Indeterminate true | Inspect control state | Indeterminate mode is applied |

### BottomBar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-BB-001 | Content hosting | QML | BottomBar loaded | Add child controls | Child controls are laid out in a row |
| CTRL-BB-002 | Theme binding | QML | Theme loaded | Inspect appearance | Radius, background and spacing follow theme |

### Toolbar

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-TB-001 | Domain create-mode navigation | QML/Interaction | Toolbar has actor, property and contract selections | Navigate to each domain section through the toolbar | Domain selections are cleared so the target form opens in create mode |
| CTRL-TB-002 | Booking and tool create-mode navigation | QML/Interaction | Toolbar has booking, analysis and annual selections | Navigate to those sections through the toolbar | Target selections are cleared so the target view opens in create mode |
| CTRL-TB-003 | App menu preserves current selection | QML/Interaction | App menu has existing selections | Navigate to a section through the menu | Existing selections are preserved unless the menu action explicitly clears them |

### RunLogList

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-RL-001 | Log click | QML/Interaction | Log model available | Activate log row | Run click signal wiring is available for row interaction |
| CTRL-RL-002 | Delete click | QML/Interaction | Log row available | Activate remove button | Delete click signal wiring is available for row interaction |
| CTRL-RL-003 | Payload summary | QML | Log contains payload | Inspect rendered text | Summary reflects annuals, analyses and formats |
| CTRL-RL-004 | Draft id forwarding | QML/Interaction | Clickable draft log row | Activate log row with a real mouse click | Run click signal includes the draft id |
| CTRL-RL-005 | Delegate click geometry | QML/Layout | Log model contains rows | Open run log list | Delegate rows expose a non-zero click height |
