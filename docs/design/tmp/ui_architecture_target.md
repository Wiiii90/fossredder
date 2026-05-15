# UI Zielarchitektur

Die UI soll am Ende eine schlanke Boundary-Schicht sein, die nur `core/ports` konsumiert.
Zwischenlayer, die nur weiterleiten, sollen verschwinden.

## Endstruktur

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
          DropController.h
          MainWindowContext.h
          MainWindowTrace.h
      workspace/
        WorkspaceFacade.h
      workflows/
        import/
          ImportWorkflow.h
          ImportDraftWorkflow.h
          ImportWorkflowState.h
          ImportRunStore.h
        export/
          ExportWorkflow.h
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

## Was weg soll

- `ui/presentation`
- `StateFacade`
- `StateFacadeProjection`
- `SessionModelProjection`
- `WorkspaceClone`
- `SelectionStateSync`
- `WorkspaceSnapshotProjector`
- entity-spezifische Controller als Dauerloesung

## Was bleibt

- `ui/shell` fuer App-Rahmen und Bootstrapping
- `ui/workspace` fuer die einzige Workspace-Facade
- `ui/workflows` fuer Import, Export und Analyse
- `ui/viewmodels` fuer QML-Datenformen
- `ui/state` fuer Session-, Selection-, Navigation-, Filter-, Mutation- und Status-Zustand
- `ui/adapters` fuer echte Boundary-Umsetzung
- `ui/platform` fuer Qt-/OS-nahe Dienste
- `ui/shared` fuer kleine Querschnittshelfer

## Dateimatrix

Die Zielstruktur basiert auf den bestehenden Dateien aus:

- `ui/actions`
- `ui/bootstrap`
- `ui/controllers`
- `ui/dialogs`
- `ui/export`
- `ui/import`
- `ui/models`
- `ui/observability`
- `ui/payload`
- `ui/state`
- `ui/text`
- `ui/util`
- `ui/window`

Diese Bereiche werden in die oben definierten Zielordner konsolidiert.

## Namensregeln

- `UiSettingsStore` wird zu `SettingsStore`
- `ImportState` wird zu `ImportWorkflowState`
- `LanguageController` wird zu `LanguageService`
- `FileSystemController` wird zu `FileSystemBrowser`
- `WorkspaceSnapshotProjector` wird nicht behalten

## Umsetzungsregel

QML bleibt vorerst stabil.
Die C++-Namen duerfen zuerst umziehen; Alias-Bruecken werden nur als Uebergang akzeptiert und spaeter geloescht.
