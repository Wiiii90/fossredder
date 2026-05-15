# UI Zielarchitektur

Diese Fassung ist die komprimierte Endstruktur fuer das `ui`-Modul nach der aktuellen Analyse.
Sie ist bewusst deutlich schlanker als die fruehere Skizze und vermeidet vor allem Zwischenlayer, die nur nach Architektur aussehen, aber keine eigene Verantwortung tragen.

Leitgedanke:

- `ui` konsumiert nur `core/ports`.
- `ui` wird nicht auf eine zweite Domain oder eine zweite Application umgebaut.
- QML bleibt weitgehend stabil und wird spaeter nur punktuell nachgezogen.
- Wo ein Layer nur weiterleitet, faellt er weg.
- Wo ein Layer wirklich orchestriert, bleibt er.

## Endprinzipien

- Ein einzelner Workspace-Einstiegspunkt statt entity-spezifischer Controller-Kette.
- Getrennte Workflows fuer Import, Export und Analyse.
- ViewModels sind die einzige starke QML-Datenform.
- State ist nur Session-, Selection-, Navigation-, Filter-, Mutation- und Status-Zustand.
- Adapter uebersetzen echte Boundary-Formen, nicht ganze Zwischensysteme.
- Platform enthaelt nur Qt-/OS-nahe Dienste.
- Shared bleibt klein und technisch neutral.

## Finale Zielstruktur

```text
ui/
  include/
    MainWindow.h
    ui/
      shell/
        AppActions.h
        AppContext.h
        QmlContracts.h
        QmlRuntime.h
      window/
          CloseWorkflow.h
          DropHandler.h
          MainWindowContext.h
          MainWindowTrace.h
      workspace/
        WorkspaceFacade.h
      workflows/
        import/
          ImportWorkflow.h
          ImportJobBridge.h
          ImportWorkflowState.h
          ImportRunStore.h
        export/
          ExportWorkflow.h
          ExportRunner.h
          WorkspaceSnapshot.h
        analysis/
          AnalysisWorkflow.h
      viewmodels/
        base/
          IndexedListModel.h
          RowListModel.h
        catalog/
          ActorListModel.h
          PropertyListModel.h
          ContractListModel.h
        booking/
          StatementListModel.h
          TransactionListModel.h
          TransactionFilterModel.h
        reporting/
          AnalysisListModel.h
          AnnualListModel.h
        import/
          ImportRunListModel.h
          ImportSuggestionViewModel.h
          StatementDraftViewModel.h
          TransactionDraftViewModel.h
          TransactionDraftListModel.h
        system/
          SettingsViewModel.h
      state/
        session/
          WorkspaceSessionState.h
          WorkspaceSessionModels.h
          WorkspaceSessionSelection.h
        selection/
          SelectionState.h
        navigation/
          NavigationState.h
        filters/
          FilterState.h
        mutation/
          SessionMutationState.h
        status/
          StatusState.h
      adapters/
        core/
          WorkspaceRowProjector.h
          AnalysisRequestMapper.h
          AnalysisResultMapper.h
          DraftViewMapper.h
          ImportDraftMapper.h
          ImportSuggestionMapper.h
          EntityPayloadMapper.h
          PayloadMapper.h
      platform/
        dialogs/
          FileDialogs.h
        filesystem/
          FileSystemBrowser.h
        localization/
          LanguageService.h
        settings/
          SettingsStore.h
      shared/
        config/
          Defaults.h
        observability/
          ErrorCodes.h
          Origins.h
          Trace.h
        payload/
          PayloadKeys.h
        text/
          Text.h
        util/
          CoreFacadeGuard.h
          StringConversions.h
        pch.h
```

## Was bewusst nicht mehr vorhanden ist

- keine `ui/presentation`-Zwischenschicht
- keine `StateFacade`
- keine `StateFacadeProjection`
- keine `SessionModelProjection`
- kein `WorkspaceClone`
- kein `SelectionStateSync`
- kein `WorkspaceSnapshotProjector`
- keine grosse Glue-Wand aus Projector-, Clone- und Sync-Klassen
- keine entity-spezifischen Controller als Dauerloesung

## Was die einzelnen Bereiche bedeuten

### `ui/shell`

App-Rahmen, MainWindow, QML-Kontext, globale Aktionen und Fensterverhalten.

### `ui/workspace`

Eine einzige UI-Facade fuer Workspace-CRUD, Snapshot-Zugriff und Verbindung zu den Core-Workspace-Ports.

### `ui/workflows`

Echte Use-Case-Orchestrierung:

- Import
- Export
- Analyse

Hier sitzt Workflow, nicht Weiterleitung.
Im Import- und Export-Zweig bleiben nur die dafuer wirklich benoetigten internen Helfer:

- `ImportJobBridge`
- `ImportWorkflowState`
- `ImportRunStore`
- `ExportRunner`
- `WorkspaceSnapshot`

### `ui/viewmodels`

Qt-/QML-Datenformen:

- ListModels
- Draft-Formen
- Settings-ViewModel

### `ui/state`

Nur lokaler UI-Zustand:

- Session
- Selection
- Navigation
- Filter
- Mutation
- Status

### `ui/adapters`

Echte Form-Umsetzung zwischen Core-Boundary und UI-Datenform.
Hier bleibt nur ein schlanker Adapterkern, vor allem:

- `WorkspaceRowProjector`
- `AnalysisRequestMapper`
- `AnalysisResultMapper`
- `DraftViewMapper`
- `ImportDraftMapper`
- `ImportSuggestionMapper`
- `EntityPayloadMapper`
- `PayloadMapper`

### `ui/platform`

Qt-/OS-nahe Dienste:

- Datei-Dialoge
- Filesystem
- Sprache
- Settings-Persistenz

### `ui/shared`

Kleiner Querschnitt:

- Text
- Payload Keys
- Defaults
- Observability
- String-Konvertierung
- Guard-Helfer

## Dateimatrix: Alt -> Neu

### Shell, Window, App-Rahmen

| Aktuell | Ziel |
| --- | --- |
| `ui/include/MainWindow.h` | `ui/include/MainWindow.h` |
| `ui/src/MainWindow.cpp` | `ui/src/MainWindow.cpp` |
| `ui/include/ui/actions/Actions.h` | `ui/include/ui/shell/AppActions.h` |
| `ui/src/actions/Actions.cpp` | `ui/src/shell/AppActions.cpp` |
| `ui/include/ui/bootstrap/AppContext.h` | `ui/include/ui/shell/AppContext.h` |
| `ui/src/bootstrap/AppContext.cpp` | `ui/src/shell/AppContext.cpp` |
| `ui/include/ui/bootstrap/QmlContracts.h` | `ui/include/ui/shell/QmlContracts.h` |
| `ui/include/ui/bootstrap/QmlRuntime.h` | `ui/include/ui/shell/QmlRuntime.h` |
| `ui/src/bootstrap/QmlRuntime.cpp` | `ui/src/shell/QmlRuntime.cpp` |
| `ui/include/ui/window/CloseWorkflow.h` | `ui/include/ui/shell/window/CloseWorkflow.h` |
| `ui/src/window/CloseWorkflow.cpp` | `ui/src/shell/window/CloseWorkflow.cpp` |
| `ui/include/ui/window/DropHandler.h` | `ui/include/ui/shell/window/DropHandler.h` |
| `ui/src/window/DropHandler.cpp` | `ui/src/shell/window/DropHandler.cpp` |
| `ui/include/ui/window/MainWindowContext.h` | `ui/include/ui/shell/window/MainWindowContext.h` |
| `ui/src/window/MainWindowContext.cpp` | `ui/src/shell/window/MainWindowContext.cpp` |
| `ui/include/ui/window/MainWindowTrace.h` | `ui/include/ui/shell/window/MainWindowTrace.h` |

### Workspace

`WorkspaceFacade` ersetzt die alten Entity-Controller und den Storage-Controller.

| Aktuell | Ziel |
| --- | --- |
| `ui/include/ui/controllers/ActorController.h` | `ui/include/ui/workspace/WorkspaceFacade.h` |
| `ui/src/controllers/ActorController.cpp` | `ui/src/workspace/WorkspaceFacade.cpp` |
| `ui/include/ui/controllers/PropertyController.h` | `ui/include/ui/workspace/WorkspaceFacade.h` |
| `ui/src/controllers/PropertyController.cpp` | `ui/src/workspace/WorkspaceFacade.cpp` |
| `ui/include/ui/controllers/ContractController.h` | `ui/include/ui/workspace/WorkspaceFacade.h` |
| `ui/src/controllers/ContractController.cpp` | `ui/src/workspace/WorkspaceFacade.cpp` |
| `ui/include/ui/controllers/StatementController.h` | `ui/include/ui/workspace/WorkspaceFacade.h` |
| `ui/src/controllers/StatementController.cpp` | `ui/src/workspace/WorkspaceFacade.cpp` |
| `ui/include/ui/controllers/TransactionController.h` | `ui/include/ui/workspace/WorkspaceFacade.h` |
| `ui/src/controllers/TransactionController.cpp` | `ui/src/workspace/WorkspaceFacade.cpp` |
| `ui/include/ui/controllers/AnnualController.h` | `ui/include/ui/workspace/WorkspaceFacade.h` |
| `ui/src/controllers/AnnualController.cpp` | `ui/src/workspace/WorkspaceFacade.cpp` |
| `ui/include/ui/controllers/StorageController.h` | `ui/include/ui/workspace/WorkspaceFacade.h` |
| `ui/src/controllers/StorageController.cpp` | `ui/src/workspace/WorkspaceFacade.cpp` |

### Workflows

| Aktuell | Ziel |
| --- | --- |
| `ui/include/ui/controllers/ImportController.h` | `ui/include/ui/workflows/import/ImportWorkflow.h` |
| `ui/src/controllers/ImportController.cpp` | `ui/src/workflows/import/ImportWorkflow.cpp` |
| `ui/include/ui/controllers/DraftController.h` | `ui/include/ui/workflows/import/ImportWorkflow.h` |
| `ui/src/controllers/DraftController.cpp` | `ui/src/workflows/import/ImportWorkflow.cpp` |
| `ui/include/ui/controllers/ExportController.h` | `ui/include/ui/workflows/export/ExportWorkflow.h` |
| `ui/src/controllers/ExportController.cpp` | `ui/src/workflows/export/ExportWorkflow.cpp` |
| `ui/include/ui/controllers/AnalysisController.h` | `ui/include/ui/workspace/WorkspaceFacade.h` + `ui/include/ui/workflows/analysis/AnalysisWorkflow.h` |
| `ui/src/controllers/AnalysisController.cpp` | `ui/src/workspace/WorkspaceFacade.cpp` + `ui/src/workflows/analysis/AnalysisWorkflow.cpp` |
| `ui/include/ui/import/ImportJobBridge.h` | `ui/include/ui/workflows/import/ImportJobBridge.h` |
| `ui/src/import/ImportJobBridge.cpp` | `ui/src/workflows/import/ImportJobBridge.cpp` |
| `ui/include/ui/import/ImportRunStore.h` | `ui/include/ui/workflows/import/ImportRunStore.h` |
| `ui/src/import/ImportRunStore.cpp` | `ui/src/workflows/import/ImportRunStore.cpp` |
| `ui/include/ui/import/ImportState.h` | `ui/include/ui/workflows/import/ImportWorkflowState.h` |
| `ui/src/import/ImportState.cpp` | `ui/src/workflows/import/ImportWorkflowState.cpp` |
| `ui/include/ui/export/ExportRunner.h` | `ui/include/ui/workflows/export/ExportRunner.h` |
| `ui/src/export/ExportRunner.cpp` | `ui/src/workflows/export/ExportRunner.cpp` |
| `ui/include/ui/export/WorkspaceSnapshot.h` | `ui/include/ui/workflows/export/WorkspaceSnapshot.h` |
| `ui/src/export/WorkspaceSnapshot.cpp` | `ui/src/workflows/export/WorkspaceSnapshot.cpp` |

### ViewModels

| Aktuell | Ziel |
| --- | --- |
| `ui/include/ui/models/IndexedListModel.h` | `ui/include/ui/viewmodels/base/IndexedListModel.h` |
| `ui/include/ui/models/RowListModel.h` | `ui/include/ui/viewmodels/base/RowListModel.h` |
| `ui/include/ui/models/ActorList.h` | `ui/include/ui/viewmodels/catalog/ActorListModel.h` |
| `ui/src/models/ActorList.cpp` | `ui/src/viewmodels/catalog/ActorListModel.cpp` |
| `ui/include/ui/models/PropertyList.h` | `ui/include/ui/viewmodels/catalog/PropertyListModel.h` |
| `ui/src/models/PropertyList.cpp` | `ui/src/viewmodels/catalog/PropertyListModel.cpp` |
| `ui/include/ui/models/ContractList.h` | `ui/include/ui/viewmodels/catalog/ContractListModel.h` |
| `ui/src/models/ContractList.cpp` | `ui/src/viewmodels/catalog/ContractListModel.cpp` |
| `ui/include/ui/models/StatementList.h` | `ui/include/ui/viewmodels/booking/StatementListModel.h` |
| `ui/src/models/StatementList.cpp` | `ui/src/viewmodels/booking/StatementListModel.cpp` |
| `ui/include/ui/models/TransactionList.h` | `ui/include/ui/viewmodels/booking/TransactionListModel.h` |
| `ui/src/models/TransactionList.cpp` | `ui/src/viewmodels/booking/TransactionListModel.cpp` |
| `ui/include/ui/models/TransactionFilter.h` | `ui/include/ui/viewmodels/booking/TransactionFilterModel.h` |
| `ui/src/models/TransactionFilter.cpp` | `ui/src/viewmodels/booking/TransactionFilterModel.cpp` |
| `ui/include/ui/models/AnalysisList.h` | `ui/include/ui/viewmodels/reporting/AnalysisListModel.h` |
| `ui/src/models/AnalysisList.cpp` | `ui/src/viewmodels/reporting/AnalysisListModel.cpp` |
| `ui/include/ui/models/AnnualList.h` | `ui/include/ui/viewmodels/reporting/AnnualListModel.h` |
| `ui/src/models/AnnualList.cpp` | `ui/src/viewmodels/reporting/AnnualListModel.cpp` |
| `ui/include/ui/models/ImportRunList.h` | `ui/include/ui/viewmodels/import/ImportRunListModel.h` |
| `ui/src/models/ImportRunList.cpp` | `ui/src/viewmodels/import/ImportRunListModel.cpp` |
| `ui/include/ui/models/ExportRunList.h` | `ui/include/ui/viewmodels/export/ExportRunListModel.h` |
| `ui/src/models/ExportRunList.cpp` | `ui/src/viewmodels/export/ExportRunListModel.cpp` |
| `ui/include/ui/models/ImportSuggestion.h` | `ui/include/ui/viewmodels/import/ImportSuggestionViewModel.h` |
| `ui/include/ui/models/StatementDraft.h` | `ui/include/ui/viewmodels/import/StatementDraftViewModel.h` |
| `ui/src/models/StatementDraft.cpp` | `ui/src/viewmodels/import/StatementDraftViewModel.cpp` |
| `ui/include/ui/models/TransactionDraft.h` | `ui/include/ui/viewmodels/import/TransactionDraftViewModel.h` |
| `ui/include/ui/models/TransactionDraftList.h` | `ui/include/ui/viewmodels/import/TransactionDraftListModel.h` |
| `ui/src/models/TransactionDraftList.cpp` | `ui/src/viewmodels/import/TransactionDraftListModel.cpp` |

### State

| Aktuell | Ziel |
| --- | --- |
| `ui/include/ui/state/SessionStore.h` | `ui/include/ui/state/session/WorkspaceSessionState.h` |
| `ui/src/state/SessionStore.cpp` | `ui/src/state/session/WorkspaceSessionState.cpp` |
| `ui/include/ui/state/SessionModels.h` | `ui/include/ui/state/session/WorkspaceSessionModels.h` |
| `ui/src/state/SessionModels.cpp` | `ui/src/state/session/WorkspaceSessionModels.cpp` |
| `ui/include/ui/state/SessionSelection.h` | `ui/include/ui/state/session/WorkspaceSessionSelection.h` |
| `ui/src/state/SessionSelection.cpp` | `ui/src/state/session/WorkspaceSessionSelection.cpp` |
| `ui/include/ui/state/SelectionState.h` | `ui/include/ui/state/selection/SelectionState.h` |
| `ui/src/state/SelectionState.cpp` | `ui/src/state/selection/SelectionState.cpp` |
| `ui/include/ui/state/NavigationState.h` | `ui/include/ui/state/navigation/NavigationState.h` |
| `ui/src/state/NavigationState.cpp` | `ui/src/state/navigation/NavigationState.cpp` |
| `ui/include/ui/state/FilterState.h` | `ui/include/ui/state/filters/FilterState.h` |
| `ui/src/state/FilterState.cpp` | `ui/src/state/filters/FilterState.cpp` |
| `ui/include/ui/state/SessionMutationState.h` | `ui/include/ui/state/mutation/SessionMutationState.h` |
| `ui/src/state/SessionMutationState.cpp` | `ui/src/state/mutation/SessionMutationState.cpp` |
| `ui/include/ui/state/StatusState.h` | `ui/include/ui/state/status/StatusState.h` |

### Entfernte Glue-Bausteine

Diese Dateien haben im Endbild keinen eigenen Platz mehr:

- `ui/include/ui/state/StateFacade.h`
- `ui/src/state/StateFacade.cpp`
- `ui/include/ui/state/StateFacadeProjection.h`
- `ui/src/state/StateFacadeProjection.cpp`
- `ui/include/ui/state/SessionModelProjection.h`
- `ui/src/state/SessionModelProjection.cpp`
- `ui/include/ui/state/WorkspaceClone.h`
- `ui/include/ui/state/SelectionStateSync.h`
- `ui/src/state/SelectionStateSync.cpp`

### Adapter und Mappers

| Aktuell | Ziel |
| --- | --- |
| `ui/include/ui/analysis/AnalysisInputMapper.h` | `ui/include/ui/adapters/core/AnalysisRequestMapper.h` |
| `ui/src/analysis/AnalysisInputMapper.cpp` | `ui/src/adapters/core/AnalysisRequestMapper.cpp` |
| `ui/include/ui/analysis/AnalysisPayloadMapper.h` | `ui/include/ui/adapters/core/AnalysisResultMapper.h` |
| `ui/src/analysis/AnalysisPayloadMapper.cpp` | `ui/src/adapters/core/AnalysisResultMapper.cpp` |
| `ui/include/ui/import/ImportDraftMapper.h` | `ui/include/ui/adapters/core/ImportDraftMapper.h` |
| `ui/src/import/ImportDraftMapper.cpp` | `ui/src/adapters/core/ImportDraftMapper.cpp` |
| `ui/include/ui/import/DraftViewMapper.h` | `ui/include/ui/adapters/core/DraftViewMapper.h` |
| `ui/src/import/DraftViewMapper.cpp` | `ui/src/adapters/core/DraftViewMapper.cpp` |
| `ui/include/ui/import/ImportSuggestionService.h` | `ui/include/ui/adapters/core/ImportSuggestionMapper.h` |
| `ui/src/import/ImportSuggestionService.cpp` | `ui/src/adapters/core/ImportSuggestionMapper.cpp` |
| `ui/include/ui/adapters/core/WorkspaceRowProjector.h` | `ui/include/ui/adapters/core/WorkspaceRowProjector.h` |
| `ui/src/adapters/core/WorkspaceRowProjector.cpp` | `ui/src/adapters/core/WorkspaceRowProjector.cpp` |
| `ui/include/ui/payload/EntityPayloadMapper.h` | `ui/include/ui/adapters/core/EntityPayloadMapper.h` |
| `ui/include/ui/payload/PayloadMapper.h` | `ui/include/ui/adapters/core/PayloadMapper.h` |
| `ui/include/ui/payload/PayloadKeys.h` | `ui/include/ui/shared/payload/PayloadKeys.h` |

### Platform

| Aktuell | Ziel |
| --- | --- |
| `ui/include/ui/dialogs/FileDialogs.h` | `ui/include/ui/platform/dialogs/FileDialogs.h` |
| `ui/src/dialogs/FileDialogs.cpp` | `ui/src/platform/dialogs/FileDialogs.cpp` |
| `ui/include/ui/controllers/FileSystemController.h` | `ui/include/ui/platform/filesystem/FileSystemBrowser.h` |
| `ui/src/controllers/FileSystemController.cpp` | `ui/src/platform/filesystem/FileSystemBrowser.cpp` |
| `ui/include/ui/controllers/LanguageController.h` | `ui/include/ui/platform/localization/LanguageService.h` |
| `ui/src/controllers/LanguageController.cpp` | `ui/src/platform/localization/LanguageService.cpp` |
| `ui/include/ui/controllers/SettingsController.h` | `ui/include/ui/viewmodels/system/SettingsViewModel.h` + `ui/include/ui/platform/settings/SettingsStore.h` |
| `ui/src/controllers/SettingsController.cpp` | `ui/src/viewmodels/system/SettingsViewModel.cpp` + `ui/src/platform/settings/SettingsStore.cpp` |

### Shared

| Aktuell | Ziel |
| --- | --- |
| `ui/include/ui/config/Defaults.h` | `ui/include/ui/shared/config/Defaults.h` |
| `ui/include/ui/observability/ErrorCodes.h` | `ui/include/ui/shared/observability/ErrorCodes.h` |
| `ui/include/ui/observability/Origins.h` | `ui/include/ui/shared/observability/Origins.h` |
| `ui/include/ui/observability/Trace.h` | `ui/include/ui/shared/observability/Trace.h` |
| `ui/include/ui/text/Text.h` | `ui/include/ui/shared/text/Text.h` |
| `ui/include/ui/util/CoreFacadeGuard.h` | `ui/include/ui/shared/util/CoreFacadeGuard.h` |
| `ui/include/ui/util/StringConversions.h` | `ui/include/ui/shared/util/StringConversions.h` |
| `ui/include/ui/pch.h` | `ui/include/ui/shared/pch.h` |

## Namensentscheidungen

### `ImportState`

Der Name ist zu generisch fuer das Zielbild.
Er wird zu `ImportWorkflowState`, damit klar ist, dass es um Workflow-Zustand und nicht um einen allgemeinen UI-Status geht.

### `ImportRunStore`

Der Name ist praezise genug und darf bleiben.
Er beschreibt den Zweck klar und kollidiert nicht mit der restlichen Architektur.

### `WorkspaceRowProjector`

Die zusätzliche Snapshot-Projektion wurde bewusst verworfen.
Die row-basierte Projektion bleibt bei `WorkspaceRowProjector`, die Workspace-Boundary bei `WorkspaceFacade` und die direkte UI-Form bei den Session-Models und Mappers.

### `SettingsController`

Der Name wird aufgeteilt:

- `SettingsViewModel` fuer QML und lokale UI-Zustandshalter
- `SettingsStore` fuer Persistenz

### `SettingsStore`

Der persistente Store fuer UI-Einstellungen heisst bewusst ohne `Ui`-Praefix einfach `SettingsStore`.
Das ist der finale Name im Zielbild.

## Wichtige Architekturregeln

- Keine UI-Domain.
- Keine UI-Application als zweites Backend.
- Keine Controller, die nur Core-Aufrufe durchreichen.
- Keine Clone- oder Sync-Zwischenschichten fuer Workspace-State.
- Keine grosse `StateFacadeProjection`.
- Keine generische `UiStateAlgorithms`-Schicht.
- Keine Pipeline aus drei Projektionen fuer dieselbe Information.

## Migration und Alias-Bruecken

Die QML-Schicht bleibt zunaechst stabil.
Wenn eine C++-Klasse umbenannt oder verschoben wird, koennen die QML-Elementnamen voruebergehend gleich bleiben.
So koennen wir:

- die alten C++-Namen schrittweise entfernen
- die QML-Nutzung in kleinen, kontrollierten Schritten nachziehen
- keine Code-Verluste riskieren

Das Ziel ist, Alias-Bruecken nur so lange wie noetig zu behalten.
Sobald QML nicht mehr davon abhaengt, werden sie geloescht.

## Zielbild in einem Satz

Die UI ist am Ende eine schlanke Boundary-Schicht mit einer Workspace-Facade, klaren Workflows, echten ViewModels, kleinem State, kleinen Adaptern und wenigen Platform-Services - ohne Projector-Wand, ohne Clone-Schichten und ohne Forwarder-Klassen, die nur nach viel Code aussehen.

## Endliche Umsetzungsreihenfolge

Die Reihenfolge ist bewusst geschlossen und nicht als offene Roadmap gemeint.

1. Zielordner und Zielnamen einziehen.
1. `SettingsController`, `ImportState`, `LanguageController`, `FileSystemController` und die Shell-Dateien auf die Zielnamen verschieben.
1. Entity-spezifische Controller in `WorkspaceFacade` zusammenziehen.
1. `ImportController`, `DraftController`, `ExportController` und den berechnenden Teil von `AnalysisController` in echte Workflows aufloesen.
1. `SessionStore`, `SessionModels` und `SessionSelection` in den Session-State ueberfuehren.
1. Glue-Dateien und Altordner loeschen, sobald kein Consumer mehr davon abhaengt.

## Brueckenregel

Die QML-Schicht bleibt zunaechst stabil.
Wenn C++-Namen umziehen, koennen temporaere Alias-Bruecken im Code bleiben, bis die QML-Nutzung schrittweise nachgezogen ist.
Sie werden nicht dauerhaft gepflegt.
