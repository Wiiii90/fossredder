# UI QML Testing

## Purpose

This document captures the QML-level behavior matrix for the UI. The tests are
written against user-visible interactions and remain valid across the UI
refactor described in `docs/design/tmp/ui_architecture_target.md`.

The matrix content itself is intentionally kept intact here. This update is a
documentation alignment pass, not an in-content test redesign.

## Relationship To The UI Refactor

The UI refactor may move files and reshape internal UI layers, but the QML test
surface stays the same as long as the observable behavior remains the same.

These tests should continue to describe:

- view composition
- button and panel wiring
- selection changes
- create/update/delete flows
- view-specific state transitions

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

### ImportForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-F-001 | Source selection | QML | Form loaded | Inspect source selector | Supported import source labels are shown |
| IMP-F-002 | Strategy selection | QML | Form loaded | Inspect strategy selector | Supported statement strategy labels are shown |

### StatementDraftView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMP-D-001 | Draft return | QML/Interaction | Draft loaded | Activate return button | Draft note is written, draft is cleared, navigation moves to booking section |
| IMP-D-002 | Draft discard | QML/Interaction | Draft loaded | Activate discard button | Persisted draft is cleared, discard note is written, navigation moves to booking section |
| IMP-D-003 | Draft finalize | QML/Interaction | Draft loaded and finalize succeeds | Activate finalize button | Draft is finalized, persisted draft is cleared, finalize note is written, navigation moves to booking section |
| IMP-D-004 | Draft finalize failure | QML/Interaction | Draft loaded and finalize fails | Activate finalize button | Failure note is written and draft remains available |
| IMP-D-005 | Transaction navigation | QML/Interaction | Draft with multiple transactions | Activate previous or next transaction button | Draft transaction index changes and snapshot persistence is requested |
| IMP-D-006 | Transaction delete | QML/Interaction | Draft with more than one transaction | Activate delete transaction button | Current transaction is removed and snapshot is persisted |

## Actor

### ActorView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-V-001 | Container mount | QML | App context and theme available | Open ActorView | `ActorForm` is filled with app context and theme |
| ACT-V-002 | Toolbar create shortcut | QML/Interaction | Actor section active and edit mode available | Click the toolbar actor create action | Actor form switches to create mode and clears selection |

### ActorForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ACT-001 | Create actor | QML/Interaction | No selected actor | Enter name and click Create | `saveActor("", ...)` is called, form clears, selected actor id becomes returned id |
| ACT-002 | Read actor state | QML/Interaction | Selected actor with name, type, description | Open form in edit mode | Fields reflect selected actor state |
| ACT-003 | Update actor | QML/Interaction | Selected actor with modified fields | Change name or aliases and click Update | `saveActor(current.id, ...)` is called and saved state is refreshed |
| ACT-004 | Delete actor | QML/Interaction | Selected actor with valid id | Click Delete | `deleteActor(id)` is called and selection advances deterministically |
| ACT-005 | Alias add | QML/Interaction | Create or edit mode with alias input available | Activate `+` alias button | Alias is trimmed, added once, input is cleared |
| ACT-006 | Alias delete | QML/Interaction | At least one alias exists | Activate `-` alias button | Alias is removed and index updates deterministically |
| ACT-007 | Contract selection | QML/Interaction | Selected actor with contract rows | Toggle contract checkboxes | Selected contract ids are updated in the form state |
| ACT-008 | Actor navigation | QML/Interaction | Actor rows available | Click Prev or Next | Selected actor id moves to adjacent actor |
| ACT-009 | Create-mode shortcut button | QML/Interaction | Edit mode with a selected actor | Click bottom-bar `+` button | Selection clears and form switches to create mode |

## Property

### PropertyView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-V-001 | Container mount | QML | App context and theme available | Open PropertyView | `PropertyForm` is filled with app context and theme |
| PROP-V-002 | Toolbar create shortcut | QML/Interaction | Property section active and edit mode available | Click the toolbar property create action | Property form switches to create mode and clears selection |

### PropertyForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PROP-001 | Create property | QML/Interaction | No selected property | Enter name and click Create | `saveProperty("", ...)` is called, form clears, selected property id becomes returned id |
| PROP-002 | Read property state | QML/Interaction | Selected property with name and aliases | Open form in edit mode | Fields reflect selected property state |
| PROP-003 | Update property | QML/Interaction | Selected property with modified fields | Change name or aliases and click Update | `saveProperty(current.id, ...)` is called and saved state is refreshed |
| PROP-004 | Delete property | QML/Interaction | Selected property with valid id | Click Delete | `deleteProperty(id)` is called and selection advances deterministically |
| PROP-005 | Alias add | QML/Interaction | Create or edit mode with alias input available | Activate `+` alias button | Alias is trimmed, added once, input is cleared |
| PROP-006 | Alias delete | QML/Interaction | At least one alias exists | Activate `-` alias button | Alias is removed and index updates deterministically |
| PROP-007 | Contract selection | QML/Interaction | Selected property with contract rows | Toggle contract checkboxes | Selected contract ids are updated in the form state |
| PROP-008 | Property navigation | QML/Interaction | Property rows available | Click Prev or Next | Selected property id moves to adjacent property |
| PROP-009 | Create-mode shortcut button | QML/Interaction | Edit mode with a selected property | Click bottom-bar `+` button | Selection clears and form switches to create mode |

## Contract

### ContractView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-V-001 | Container mount | QML | App context and theme available | Open ContractView | `ContractForm` is filled with app context and theme |
| CON-V-002 | Toolbar create shortcut | QML/Interaction | Contract section active and edit mode available | Click the toolbar contract create action | Contract form switches to create mode and clears selection |

### ContractForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CON-001 | Create contract | QML/Interaction | No selected contract | Enter name and click Create | `saveContract("", ...)` is called, form clears, selected contract id becomes returned id |
| CON-002 | Read contract state | QML/Interaction | Selected contract with type, actors, properties, aliases | Open form in edit mode | Fields and selection panels reflect selected contract state |
| CON-003 | Update contract | QML/Interaction | Selected contract with modified fields | Change name, type, selections or aliases and click Update | `saveContract(current.id, ...)` is called and saved state is refreshed |
| CON-004 | Delete contract | QML/Interaction | Selected contract with valid id | Click Delete | `deleteContract(id)` is called and selection advances deterministically |
| CON-005 | Alias add | QML/Interaction | Create or edit mode with alias input available | Activate `+` alias button | Alias is trimmed, added once, input is cleared |
| CON-006 | Alias delete | QML/Interaction | At least one alias exists | Activate `-` alias button | Alias is removed and index updates deterministically |
| CON-007 | Actor selection | QML/Interaction | Actor rows available | Toggle contract actor checkboxes | Selected actor ids are updated in the form state |
| CON-008 | Property selection | QML/Interaction | Property rows available | Toggle contract property checkboxes | Selected property ids are updated in the form state |
| CON-009 | Contract navigation | QML/Interaction | Contract rows available | Click Prev or Next | Selected contract id moves to adjacent contract |
| CON-010 | Create-mode shortcut button | QML/Interaction | Edit mode with a selected contract | Click bottom-bar `+` button | Selection clears and form switches to create mode |

## Annual

### AnnualView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-V-001 | Container mount | QML | App context and theme available | Open AnnualView | `AnnualForm` is filled with app context and theme |

### AnnualForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANN-001 | Create annual | QML/Interaction | No selected annual, valid year entered | Enter annual name and year, click Create | `saveAnnual("", ...)` is called, selected annual id becomes returned id |
| ANN-002 | Read annual state | QML/Interaction | Selected annual with loaded analyses and transactions | Open form in edit mode | Annual fields and derived panels reflect selected annual state |
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

## Booking

### BookingView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-V-001 | Container mount | QML | App context and theme available | Open BookingView | `BookingStatementView` is filled with app context and theme |

### BookingStatementView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-001 | Create statement | QML/Interaction | No selected statement and valid name entered | Enter name and click Create | `addStatement()` and `addTransactions()` are called, selected statement id becomes returned id |
| BKG-002 | Read statement state | QML/Interaction | Selected statement with transactions loaded | Open in edit mode | Statement and transaction fields reflect selected statement state |
| BKG-003 | Update statement | QML/Interaction | Selected statement with modified name or transaction data | Change fields and click Update | `updateStatement()` and `updateTransaction()` are called for active data |
| BKG-004 | Delete statement | QML/Interaction | Selected statement with valid id | Click Delete | `deleteStatement(id)` is called and selection advances deterministically |
| BKG-005 | Create transaction in create mode | QML/Interaction | Create mode with a draft statement | Click transaction add | A new draft transaction is inserted after current one |
| BKG-006 | Delete transaction in create mode | QML/Interaction | Create mode with multiple transactions | Click transaction delete | Current draft transaction is removed if more than one remains |
| BKG-007 | Create mode navigation | QML/Interaction | Multiple draft transactions | Click Prev or Next transaction | Current draft transaction index changes |
| BKG-008 | Statement navigation | QML/Interaction | Statement rows available | Click Prev page or Next page | Selected statement id moves to adjacent statement |

### BookingStatementPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-P-001 | Statement name edit | QML/Interaction | Panel loaded | Edit statement name | Parent view receives updated name |
| BKG-P-002 | Transaction add request | QML/Interaction | Panel loaded | Activate add transaction button | Add request is forwarded |
| BKG-P-003 | Transaction delete request | QML/Interaction | Panel loaded with removable transaction | Activate remove transaction button | Delete request is forwarded |

### BookingTransactionView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-T-001 | Transaction field edit | QML/Interaction | Transaction data loaded | Edit transaction fields | Transaction draft updates in parent view |
| BKG-T-002 | Actor selection | QML/Interaction | Actor rows available | Select actor in transaction | Transaction actor id updates |
| BKG-T-003 | Contract selection | QML/Interaction | Contract rows available | Select contract in transaction | Transaction contract id updates |
| BKG-T-004 | Property selection | QML/Interaction | Property rows available | Select property in transaction | Transaction property ids update |
| BKG-T-005 | Allocatable toggle | QML/Interaction | Transaction loaded | Toggle allocatable | Transaction allocatable flag updates |

### BookingTransactionActorPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TA-001 | Actor selection list | QML | Actor rows available | Select an actor | Selected actor id is forwarded |

### BookingTransactionContractPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TC-001 | Contract selection list | QML | Contract rows available | Select a contract | Selected contract id is forwarded |

### BookingTransactionPropertyPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TP-001 | Property selection list | QML | Property rows available | Select a property | Selected property ids are forwarded |

### BookingTransactionAllocatablePanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| BKG-TA-002 | Allocatable mode | QML | Transaction loaded | Change allocatable mode | Mode is forwarded to transaction draft |

## Analysis

### AnalysisView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ANL-V-001 | Container refresh | QML | App context and theme available | Show view with selection | `AnalysisForm.refreshFromSelection()` is called when view becomes visible |

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

## Export

### ExportView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-001 | Default directory | QML/Interaction | Settings or file system default directory available | Open ExportView | Export form starts with default directory |
| EXP-002 | Clear export form | QML/Interaction | Export controller idle | Click Clear | Form, panel and active run state are cleared |
| EXP-003 | Cancel export | QML/Interaction | Export controller running | Click Cancel | `cancelExport()` is called |
| EXP-004 | Pause or resume export | QML/Interaction | Export controller running | Click Pause or Resume | `togglePause()` is called |
| EXP-005 | Start export payload | QML/Interaction | Export entries and target directory present | Click Start | `exportDataWithPayload()` is called with normalized payload and correct item count |
| EXP-006 | Browse directory | QML/Interaction | Export form loaded | Click Browse | Browse request is emitted from form and routed to actions |
| EXP-007 | Directory selection | QML/Interaction | Actions emits selected directory | Select directory | Export form target directory is updated |
| EXP-008 | Settings sync | QML/Interaction | Settings default directory or archive format changes | Change settings values | Export form updates its directory and archive format |

### ExportForm

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-F-001 | Target directory edit | QML | Form loaded | Edit target directory | Target directory property updates |
| EXP-F-002 | Archive format edit | QML | Form loaded | Change archive selector | Package format index updates |
| EXP-F-003 | Browse request | QML | Form loaded | Click Browse | Browse request signal is emitted |

### ExportPanel

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| EXP-P-001 | Add annual entry | QML/Interaction | Annual rows available | Activate add entry button for annual | Annual entry is added once |
| EXP-P-002 | Add analysis entry | QML/Interaction | Analysis rows available | Activate add entry button for analysis | Analysis entry is added once |
| EXP-P-003 | Remove entry | QML/Interaction | At least one export entry present | Activate remove entry button | Entry is removed deterministically |
| EXP-P-004 | Update annual entry | QML/Interaction | Annual entry present | Select different annual | Entry updates to selected annual and linked analyses |
| EXP-P-005 | Collapse annual entry | QML/Interaction | Annual entry present | Toggle collapse button | Collapsed state flips deterministically |
| EXP-P-006 | Update analysis export type | QML/Interaction | Standalone or nested analysis present | Change export type selector | Export type is normalized and updated |
| EXP-P-007 | Load items | QML/Interaction | Serialized items available | Load items | Entries reconstruct according to payload rows |
| EXP-P-008 | Export items mapping | QML/Interaction | Entries present | Read export items | Output payload matches current entries |

## Settings

### SettingsView

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SET-V-001 | Container mount | QML | App context and theme available | Open SettingsView | Correct category component is loaded |
| SET-V-002 | Category navigation | QML/Interaction | Settings category available | Click Prev or Next | Navigation category changes within bounds |
| SET-V-003 | Save settings | QML/Interaction | Settings controller available | Click Update | Settings are saved and language is applied |
| SET-V-004 | Reset settings | QML/Interaction | Settings controller available | Click Default | Category resets and settings return to defaults |

### SettingsGeneral

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SET-G-001 | Language selection | QML/Interaction | Available languages loaded | Change language combo box | Settings language updates to selected code |
| SET-G-002 | Invalid language selection | QML/Interaction | Unavailable language present | Select unavailable language | Selection remains on a valid available language |
| SET-G-003 | Current language sync | QML/Interaction | Settings language changes externally | Refresh view bindings | Combo box reflects current controller language |

### SettingsImport

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SET-I-001 | Default import path edit | QML/Interaction | SettingsImport loaded | Edit default path field | `importDefaultPath` updates |
| SET-I-002 | Browse import path | QML/Interaction | SettingsImport loaded | Click Browse | `browseImportPdf()` is called |
| SET-I-003 | Import file signal wiring | QML/Interaction | Actions emits `importFileSelected` | Emit selected path | `importDefaultPath` is updated from signal |

### SettingsExport

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SET-E-001 | Default output directory edit | QML/Interaction | SettingsExport loaded | Edit default directory field | `exportDefaultDirectory` updates |
| SET-E-002 | Archive format selection | QML/Interaction | SettingsExport loaded | Change archive format combo | `exportArchiveFormat` updates |
| SET-E-003 | Include formulas toggle | QML/Interaction | SettingsExport loaded | Toggle formulas checkbox | `exportIncludeFormulas` updates |
| SET-E-004 | Browse/export directory signal wiring | QML/Interaction | SettingsExport loaded | Click Browse or emit `exportDirectorySelected` | Browse action is called and selected path is applied |

### SettingsMiscellaneous

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SET-M-001 | Toolbar visibility binding | QML/Interaction | SettingsMiscellaneous loaded | Toggle toolbar visibility flags | Settings toolbar flags update deterministically |

## Workflow and E2E Candidates

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| WF-IMP-001 | Draft finalize flow | QML/Interaction | Import draft available | Finalize draft | Statement is created, draft is cleared and log navigation target is stable |
| WF-IMP-002 | Draft discard flow | QML/Interaction | Import draft available | Discard draft | Draft persistence is cleared and import navigation returns to idle state |
| WF-EXP-001 | Export payload flow | QML/Interaction | Export entries configured | Start export | Payload contains selected objects and current export configuration |
| WF-BKG-001 | Statement transaction flow | QML/Interaction | Statement with editable transaction loaded | Update transaction and save statement | Statement and transaction state stay synchronized |
| WF-ANL-001 | Analysis preview flow | QML/Interaction | Filters changed | Apply filter changes | Preview refresh follows current filter state |

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

### RunLogList

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| CTRL-RL-001 | Log click | QML/Interaction | Log model available | Activate log row | Run click signal wiring is available for row interaction |
| CTRL-RL-002 | Delete click | QML/Interaction | Log row available | Activate remove button | Delete click signal wiring is available for row interaction |
| CTRL-RL-003 | Payload summary | QML | Log contains payload | Inspect rendered text | Summary reflects annuals, analyses and formats |

## Test File Layout

| Folder | Purpose |
|---|---|
| `ui/tests/qml/common` | Shared control and component tests |
| `ui/tests/qml/actor` | Actor view and form tests |
| `ui/tests/qml/property` | Property view and form tests |
| `ui/tests/qml/contract` | Contract view and form tests |
| `ui/tests/qml/booking` | Booking view and statement tests |

## Support Files

| File | Purpose |
|---|---|
| `ui/tests/qml/Lookup.js` | Recursive object lookup helper for QML tests |
| `ui/tests/qml/qmltests.cpp` | Quick Test bootstrap for UI QML tests |

## Test Support Files

### Lookup.js

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SUP-001 | Recursive object lookup | QML test support | Test tree contains nested objects | Search by objectName | Matching object is returned from nested content or special item slots |

### qmltests.cpp

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| SUP-002 | Quick Test bootstrap | C++ test support | QML test binary starts | Initialize engine | UI QML imports and runtime registration are configured before tests run |
