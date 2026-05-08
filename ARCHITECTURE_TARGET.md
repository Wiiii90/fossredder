# FOSSredder Clean Architecture Target

## Purpose of this document

This file defines the **target architecture** for the current codebase and is intended to be used as a **GPT refactoring guide**.

It should be concrete enough that an automated coding agent can:
- move files
- rename files
- update namespaces
- update include paths
- record what is already done
- record what is still open

Short-term breakage is acceptable during the refactor.
Long-term architectural consistency is the goal.

---

## Hard decisions

### 1. No `contracts` tree
There will be **no** `contracts` folder.

### 2. No duplicated feature tree under multiple roots
There will be **no** second `analysis/import/export/...` feature tree under another top-level grouping such as `contracts`.

### 3. `core/models` is only for enterprise business rules
`core/models` must contain only:
- domain entities
- value objects

### 4. `core/application` is only for application business rules
`core/application` must contain:
- use cases
- workflow orchestration
- request/result/impact/state types that belong to those use cases

It must **not** contain controllers, presenters, repository implementations, or framework adapters.

### 5. Ports are explicit
All interfaces that application code depends on must live in:
- `core/ports/*`

### 6. Adapters stay outside core
These remain outside `core`:
- `ui`
- `persistence`
- `services`
- `debug`
- `app`

---

## Layer model for this codebase

## 1. Enterprise business rules
Folders:
- `core/models/domain`
- `core/models/valueobjects`

Contains:
- business entities
- value objects
- no workflow DTOs
- no request/result types
- no persistence impact types
- no workspace orchestration state

Namespace target:
- `core::domain`

## 2. Application business rules
Folder:
- `core/application`

Contains:
- use cases
- workflow state
- workflow requests
- workflow results
- orchestration services
- application-level impact/result models

Namespace target:
- `core::application`

## 3. Ports
Folder:
- `core/ports`

Contains only interfaces / boundaries:
- repository ports
- storage ports
- external service ports
- presenter ports when needed

Namespace target:
- `core::ports::*`

## 4. Interface adapters
Folders:
- `ui`
- `persistence`
- `services`
- `debug`

These implement or consume ports.

## 5. Frameworks and drivers
Folders:
- `app`
- Qt / QML
- SQLite
- Poppler
- OpenCV
- Tesseract
- filesystem / OS runtime

---

## Target folder structure

```text
core/
  models/
    domain/
      Actor.h
      Analysis.h
      Annual.h
      Contract.h
      Property.h
      Statement.h
      Transaction.h
    valueobjects/
      Alias.h
      AliasUsage.h

  application/
    workspace/
      WorkspaceState.h
      WorkspaceFacade.h
      WorkspaceSession.h
      CatalogService.h
      StateHydrator.h
      StateProjector.h
    analysis/
      RunAnalysis.h
      RunAnalysisRequest.h
      RunAnalysisResult.h
      ComposeAnalysisRequest.h
      PropertyMetricsService.h
    import/
      ImportStatement.h
      ImportStatementRequest.h
      ImportStatementResult.h
      FinalizeDraft.h
      FinalizeDraftRequest.h
      FinalizeDraftResult.h
      StatementDraft.h
      TransactionDraft.h
      ImportLog.h
      DraftLinkingService.h
    export/
      ExportData.h
      ExportDataRequest.h
      ExportDataResult.h
      ExportAnalyses.h
      ExportLog.h
    storage/
      SaveWorkspace.h
      SaveWorkspaceRequest.h
      SaveWorkspaceResult.h
      OpenWorkspace.h
      OpenWorkspaceRequest.h
      OpenWorkspaceResult.h
      WorkspaceStorageService.h
      DeletionImpact.h

  ports/
    repositories/
      IActorRepository.h
      IAnalysisRepository.h
      IAnnualRepository.h
      IContractRepository.h
      IExportLogRepository.h
      IImportLogRepository.h
      IPropertyRepository.h
      IStatementDraftRepository.h
      IStatementRepository.h
      ITransactionDraftRepository.h
      ITransactionRepository.h
    storage/
      IRegistry.h
      IWorkspaceStore.h
    services/
      IPopplerService.h
      IOpenCvService.h
      ITesseractService.h
    presenters/
      IWorkspacePresenter.h
      IImportPresenter.h
      IExportPresenter.h
      IAnalysisPresenter.h

  jobs/
    JobSystem.h
    Scheduler.h
    JobTypes.h
    ImportJobSpec.h

  errors/
    ErrorCodes.h
    ErrorEvent.h
    ErrorEventFactory.h
    ErrorReporterRegistry.h
    ErrorReporting.h
    IErrorReporter.h
```

Adapters outside core:

```text
ui/
  controllers/
  state/
  models/
  analysis/
  import/
  export/
  bootstrap/
  dialogs/
  payload/
  observability/
  window/
  util/

persistence/
  store/
  repositories/
  sqlite/

services/
  poppler/
  opencv/
  tesseract/

debug/
  reporters/
  runtime/

app/
  composition/
  startup/
```

Notes:
- The current repository does not yet match this split.
- The names above are the long-term target, not a requirement to rename everything in one step.

### Concrete tree check used for this document

This target was mirrored against the current repository structure, including concrete `.h` and `.cpp` files in these areas:
- `core/include/core/application/*`
- `core/src/application/*`
- `core/include/core/storage/*`
- `core/src/storage/*`
- `core/include/core/import/*`
- `core/src/import/*`
- `core/include/core/export/*`
- `core/src/export/*`
- `core/include/core/repositories/*`
- `core/include/core/models/*`
- `core/include/core/analysis/*`
- `core/src/analysis/*`
- `ui/include/ui/*`
- `ui/src/*`
- `persistence/include/persistence/*`
- `persistence/src/*`
- `services/opencv/*`
- `services/poppler/*`
- `services/tesseract/*`
- `api/include/api/*`
- `debug/include/debug/*`
- `debug/src/*`
- `app/src/*`

This means the mappings below are not abstract examples; they are based on the current codebase layout.

---

## How current concrete folders map to the target

## `core/models`
Current problem:
- contains domain entities
- contains value objects
- contains workflow DTOs
- contains workspace/application state
- contains storage result types

Target:
- only `domain` and `valueobjects`

## `core/application`
Current problem:
- valid application logic exists here
- but file naming is inconsistent
- some files are really workspace use cases
- some are analysis use cases
- some are import use cases

Target:
- remains the application business rules layer
- split by workflow responsibility inside `core/application`

## `core/storage`
Current problem:
- mixes interfaces and implementation
- contains both boundary abstractions and concrete orchestration

Target:
- interfaces move to `core/ports/storage`
- concrete orchestration moves to `core/application/storage`

## `core/repositories`
Current state:
- already a good home for repository abstractions

Target:
- move to `core/ports/repositories`

## `core/import`
Current state:
- mixed layer
- contains a use-case interface (`IImportStatement`)
- contains parsing internals and pipeline strategy code

Target:
- application-facing request/result/use-case types move to `core/application/import`
- parser and algorithm internals may remain in `core/import` or be renamed later to `core/importing/internal`
- port interfaces move to `core/ports/services` or `core/application/import` only when they are actual use-case boundaries

## `core/export`
Current state:
- mixed layer
- contains request/result plus orchestration and exporter implementations

Target:
- request/result/use-case types move to `core/application/export`
- exporter strategy and writer logic may remain in `core/export` temporarily, but long-term should be placed according to whether they are application logic or adapters

## `api`
Current state:
- contains interfaces to external libraries plus request/response/types

Target:
- service interfaces move into `core/ports/services`
- external library-specific request/response/types may remain in `api` only if you intentionally keep `api` as a technology boundary package
- otherwise those types move into `services/*` or `core/application/import` depending on who owns them logically

## `ui`
Current state:
- controllers, state facades, view models, payload mappers

Target:
- remains interface adapters
- this is the controller/presenter layer in practice
- it must not move into `core`

## `persistence`
Current state:
- repository implementations and SQLite store logic

Target:
- remains outbound adapter layer
- implements `core/ports/repositories` and `core/ports/storage`

## `services`
Current state:
- adapters for Poppler / OpenCV / Tesseract implementations

Target:
- remains outbound adapter layer
- implements `core/ports/services`

## `debug`
Current state:
- debug reporters and runtime tracing helpers

Target:
- remains adapter/tooling layer outside core
- may implement error reporting or diagnostic ports where needed

---

## Concrete old -> new header mapping

## Domain and value objects
- `core/include/core/models/Actor.h -> core/include/core/models/domain/Actor.h`
- `core/include/core/models/Analysis.h -> core/include/core/models/domain/Analysis.h`
- `core/include/core/models/Annual.h -> core/include/core/models/domain/Annual.h`
- `core/include/core/models/Contract.h -> core/include/core/models/domain/Contract.h`
- `core/include/core/models/Property.h -> core/include/core/models/domain/Property.h`
- `core/include/core/models/Statement.h -> core/include/core/models/domain/Statement.h`
- `core/include/core/models/Transaction.h -> core/include/core/models/domain/Transaction.h`
- `core/include/core/models/Alias.h -> core/include/core/models/valueobjects/Alias.h`
- `core/include/core/models/AliasUsage.h -> core/include/core/models/valueobjects/AliasUsage.h`

## Workspace / application state
- `core/include/core/models/AppState.h -> core/include/core/application/workspace/WorkspaceState.h`
- `core/include/core/models/DeletionImpact.h -> core/include/core/application/storage/DeletionImpact.h`
- `core/include/core/application/AppStateFacade.h -> core/include/core/application/workspace/WorkspaceFacade.h`
- `core/include/core/application/WorkspaceSession.h -> core/include/core/application/workspace/WorkspaceSession.h`
- `core/include/core/application/CatalogService.h -> core/include/core/application/workspace/CatalogService.h`
- `core/include/core/application/StateHydrator.h -> core/include/core/application/workspace/StateHydrator.h`
- `core/include/core/application/StateProjector.h -> core/include/core/application/workspace/StateProjector.h`
- `core/include/core/application/AppStateManager.h -> core/include/core/application/workspace/WorkspacePersistenceProjector.h`

## Analysis application types
- `core/include/core/models/AnalysisResult.h -> core/include/core/application/analysis/RunAnalysisResult.h`
- `core/include/core/application/AnalysisService.h -> core/include/core/application/analysis/RunAnalysis.h`
- `core/include/core/application/AnalysisRequestComposer.h -> core/include/core/application/analysis/ComposeAnalysisRequest.h`
- `core/include/core/application/PropertyMetricsService.h -> core/include/core/application/analysis/PropertyMetricsService.h`

## Import application types
- `core/include/core/import/ImportRequest.h -> core/include/core/application/import/ImportStatementRequest.h`
- `core/include/core/import/ImportResult.h -> core/include/core/application/import/ImportStatementResult.h`
- `core/include/core/import/IImportStatement.h -> core/include/core/application/import/ImportStatement.h`
- `core/include/core/models/StatementDraft.h -> core/include/core/application/import/StatementDraft.h`
- `core/include/core/models/TransactionDraft.h -> core/include/core/application/import/TransactionDraft.h`
- `core/include/core/models/ImportLog.h -> core/include/core/application/import/ImportLog.h`
- `core/include/core/application/DraftFinalizer.h -> core/include/core/application/import/FinalizeDraft.h`
- `core/include/core/import/DraftLinking.h -> core/include/core/application/import/DraftLinkingService.h`

## Export application types
- `core/include/core/export/ExportRequest.h -> core/include/core/application/export/ExportDataRequest.h`
- `core/include/core/export/ExportResult.h -> core/include/core/application/export/ExportDataResult.h`
- `core/include/core/export/ExportService.h -> core/include/core/application/export/ExportData.h`
- `core/include/core/export/AnalysisExportService.h -> core/include/core/application/export/ExportAnalyses.h`
- `core/include/core/models/ExportLog.h -> core/include/core/application/export/ExportLog.h`

## Storage ports and orchestration
- `core/include/core/storage/IStorageManager.h -> core/include/core/ports/storage/IWorkspaceStore.h`
- `core/include/core/storage/IRegistry.h -> core/include/core/ports/storage/IRegistry.h`
- `core/include/core/storage/StorageManager.h -> core/include/core/application/storage/WorkspaceStorageService.h`
- `core/include/core/storage/RepositoryBundle.h -> core/include/core/application/storage/RepositoryBundle.h`

## Repository ports
- `core/include/core/repositories/IActorRepository.h -> core/include/core/ports/repositories/IActorRepository.h`
- `core/include/core/repositories/IAnalysisRepository.h -> core/include/core/ports/repositories/IAnalysisRepository.h`
- `core/include/core/repositories/IAnnualRepository.h -> core/include/core/ports/repositories/IAnnualRepository.h`
- `core/include/core/repositories/IContractRepository.h -> core/include/core/ports/repositories/IContractRepository.h`
- `core/include/core/repositories/IExportLogRepository.h -> core/include/core/ports/repositories/IExportLogRepository.h`
- `core/include/core/repositories/IImportLogRepository.h -> core/include/core/ports/repositories/IImportLogRepository.h`
- `core/include/core/repositories/IPropertyRepository.h -> core/include/core/ports/repositories/IPropertyRepository.h`
- `core/include/core/repositories/IStatementDraftRepository.h -> core/include/core/ports/repositories/IStatementDraftRepository.h`
- `core/include/core/repositories/IStatementRepository.h -> core/include/core/ports/repositories/IStatementRepository.h`
- `core/include/core/repositories/ITransactionDraftRepository.h -> core/include/core/ports/repositories/ITransactionDraftRepository.h`
- `core/include/core/repositories/ITransactionRepository.h -> core/include/core/ports/repositories/ITransactionRepository.h`

## External service ports
- `api/include/api/poppler/IPopplerService.h -> core/include/core/ports/services/IPopplerService.h`
- `api/include/api/opencv/IOpenCvService.h -> core/include/core/ports/services/IOpenCvService.h`
- `api/include/api/tesseract/ITesseractService.h -> core/include/core/ports/services/ITesseractService.h`

## Jobs
- `core/include/core/jobs/ImportJobSpec.h -> core/include/core/jobs/ImportJobSpec.h`
- `core/include/core/jobs/JobTypes.h -> core/include/core/jobs/JobTypes.h`
- `core/include/core/jobs/JobSystem.h -> core/include/core/jobs/JobSystem.h`
- `core/include/core/jobs/Scheduler.h -> core/include/core/jobs/Scheduler.h`
- `core/src/jobs/JobManager.h -> core/include/core/jobs/JobManager.h`

## Errors
- `core/include/core/errors/ErrorCodes.h -> core/include/core/errors/ErrorCodes.h`
- `core/include/core/errors/ErrorEvent.h -> core/include/core/errors/ErrorEvent.h`
- `core/include/core/errors/IErrorReporter.h -> core/include/core/errors/IErrorReporter.h`

---

## Concrete old -> new source mapping (.cpp / helper files)

## Workspace / storage sources
- `core/src/application/AppStateFacade.cpp -> core/src/application/workspace/WorkspaceFacade.cpp`
- `core/src/application/AppStateManager.cpp -> core/src/application/workspace/WorkspacePersistenceProjector.cpp`
- `core/src/application/CatalogService.cpp -> core/src/application/workspace/CatalogService.cpp`
- `core/src/application/CatalogDraftAppliers.h -> core/src/application/workspace/CatalogDraftAppliers.h`
- `core/src/application/CatalogMutationHelpers.h -> core/src/application/workspace/CatalogMutationHelpers.h`
- `core/src/application/StateHydrator.cpp -> core/src/application/workspace/StateHydrator.cpp`
- `core/src/application/StateProjector.cpp -> core/src/application/workspace/StateProjector.cpp`
- `core/src/application/WorkspaceSession.cpp -> core/src/application/workspace/WorkspaceSession.cpp`
- `core/src/storage/StorageManager.cpp -> core/src/application/storage/WorkspaceStorageService.cpp`

## Analysis sources
- `core/src/application/AnalysisService.cpp -> core/src/application/analysis/RunAnalysis.cpp`
- `core/src/application/AnalysisRequestComposer.cpp -> core/src/application/analysis/ComposeAnalysisRequest.cpp`
- `core/src/application/PropertyMetricsService.cpp -> core/src/application/analysis/PropertyMetricsService.cpp`
- `core/src/analysis/AnalysisEngine.cpp -> core/src/application/analysis/AnalysisEngine.cpp`
- `core/src/analysis/TransactionCollector.cpp -> core/src/application/analysis/TransactionCollector.cpp`
- `core/src/analysis/OutputTypeResolver.cpp -> core/src/application/analysis/OutputTypeResolver.cpp`
- `core/src/analysis/Filter.cpp -> core/src/application/analysis/Filter.cpp`
- `core/src/analysis/FilteredTransactions.h -> core/src/application/analysis/FilteredTransactions.h`
- `core/src/analysis/strategies/CalcAnalysisStrategy.cpp -> core/src/application/analysis/strategies/CalcAnalysisStrategy.cpp`
- `core/src/analysis/strategies/PlotAnalysisStrategy.cpp -> core/src/application/analysis/strategies/PlotAnalysisStrategy.cpp`
- `core/src/analysis/strategies/TabAnalysisStrategy.cpp -> core/src/application/analysis/strategies/TabAnalysisStrategy.cpp`

## Import sources
- `core/src/application/DraftFinalizer.cpp -> core/src/application/import/FinalizeDraft.cpp`
- `core/src/import/ImportStatement.cpp -> core/src/application/import/ImportStatement.cpp`
- `core/src/import/DefaultImportStatementStrategy.cpp -> core/src/application/import/DefaultImportStatementStrategy.cpp`
- `core/src/import/DraftLinking.cpp -> core/src/application/import/DraftLinkingService.cpp`
- `core/src/import/ImportPipelineHelpers.cpp -> core/src/application/import/ImportPipelineHelpers.cpp`
- `core/src/import/ImportStrategySupport.cpp -> core/src/application/import/ImportStrategySupport.cpp`
- `core/src/import/ImportStatementStrategy.h -> core/src/application/import/ImportStatementStrategy.h`
- `core/src/import/ImportPageRequests.h -> core/src/application/import/ImportPageRequests.h`
- `core/src/import/parsing/* -> core/src/application/import/parsing/*`

## Export sources
- `core/src/export/ExportService.cpp -> core/src/application/export/ExportData.cpp`
- `core/src/export/AnalysisExportService.cpp -> core/src/application/export/ExportAnalyses.cpp`
- `core/src/export/CsvExporter.cpp -> core/src/application/export/CsvExporter.cpp`
- `core/src/export/XlsxExporter.cpp -> core/src/application/export/XlsxExporter.cpp`
- `core/src/export/PropertyContractMatrix.h -> core/src/application/export/PropertyContractMatrix.h`

## Jobs and errors sources
- `core/src/jobs/JobManager.cpp -> core/src/jobs/JobManager.cpp`
- `core/src/jobs/JobSystem.cpp -> core/src/jobs/JobSystem.cpp`
- `core/src/jobs/Scheduler.cpp -> core/src/jobs/Scheduler.cpp`
- `core/src/errors/ErrorReporterRegistry.cpp -> core/src/errors/ErrorReporterRegistry.cpp`

## Adapter sources that should stay outside core

### UI adapters
- `ui/src/controllers/ActorController.cpp -> ui/src/controllers/ActorController.cpp`
- `ui/src/controllers/AnalysisController.cpp -> ui/src/controllers/AnalysisController.cpp`
- `ui/src/controllers/AnnualController.cpp -> ui/src/controllers/AnnualController.cpp`
- `ui/src/controllers/ContractController.cpp -> ui/src/controllers/ContractController.cpp`
- `ui/src/controllers/DraftController.cpp -> ui/src/controllers/DraftController.cpp`
- `ui/src/controllers/ExportController.cpp -> ui/src/controllers/ExportController.cpp`
- `ui/src/controllers/FileSystemController.cpp -> ui/src/controllers/FileSystemController.cpp`
- `ui/src/controllers/ImportController.cpp -> ui/src/controllers/ImportController.cpp`
- `ui/src/controllers/LanguageController.cpp -> ui/src/controllers/LanguageController.cpp`
- `ui/src/controllers/PropertyController.cpp -> ui/src/controllers/PropertyController.cpp`
- `ui/src/controllers/SettingsController.cpp -> ui/src/controllers/SettingsController.cpp`
- `ui/src/controllers/StatementController.cpp -> ui/src/controllers/StatementController.cpp`
- `ui/src/controllers/StorageController.cpp -> ui/src/controllers/StorageController.cpp`
- `ui/src/controllers/TransactionController.cpp -> ui/src/controllers/TransactionController.cpp`
- `ui/src/state/StateFacade.cpp -> ui/src/state/StateFacade.cpp`
- `ui/src/state/StateFacadeProjection.cpp -> ui/src/state/StateFacadeProjection.cpp`
- `ui/src/state/SessionMutationState.cpp -> ui/src/state/SessionMutationState.cpp`
- `ui/src/analysis/AnalysisInputMapper.cpp -> ui/src/analysis/AnalysisInputMapper.cpp`
- `ui/src/analysis/AnalysisPayloadMapper.cpp -> ui/src/analysis/AnalysisPayloadMapper.cpp`
- `ui/src/import/DraftViewMapper.cpp -> ui/src/import/DraftViewMapper.cpp`
- `ui/src/import/ImportDraftMapper.cpp -> ui/src/import/ImportDraftMapper.cpp`
- `ui/src/import/ImportJobBridge.cpp -> ui/src/import/ImportJobBridge.cpp`
- `ui/src/import/ImportRunStore.cpp -> ui/src/import/ImportRunStore.cpp`
- `ui/src/import/ImportState.cpp -> ui/src/import/ImportState.cpp`
- `ui/src/import/ImportSuggestionService.cpp -> ui/src/import/ImportSuggestionService.cpp`
- `ui/src/export/AppStateSnapshot.cpp -> ui/src/export/AppStateSnapshot.cpp`
- `ui/src/export/ExportRunner.cpp -> ui/src/export/ExportRunner.cpp`

### Persistence adapters
- `persistence/src/AppStateStore.cpp -> persistence/src/store/AppStateStore.cpp`
- `persistence/src/Factory.cpp -> persistence/src/store/Factory.cpp`
- `persistence/src/SqliteDb.cpp -> persistence/src/sqlite/SqliteDb.cpp`
- `persistence/src/SqliteRegistry.cpp -> persistence/src/sqlite/SqliteRegistry.cpp`
- `persistence/src/SqliteSchema.cpp -> persistence/src/sqlite/SqliteSchema.cpp`
- `persistence/src/SqliteTransaction.cpp -> persistence/src/sqlite/SqliteTransaction.cpp`
- `persistence/src/repositories/SqliteActorRepository.cpp -> persistence/src/repositories/SqliteActorRepository.cpp`
- `persistence/src/repositories/SqliteAnalysisRepository.cpp -> persistence/src/repositories/SqliteAnalysisRepository.cpp`
- `persistence/src/repositories/SqliteAnnualRepository.cpp -> persistence/src/repositories/SqliteAnnualRepository.cpp`
- `persistence/src/repositories/SqliteConfigRepository.cpp -> persistence/src/repositories/SqliteConfigRepository.cpp`
- `persistence/src/repositories/SqliteContractRepository.cpp -> persistence/src/repositories/SqliteContractRepository.cpp`
- `persistence/src/repositories/SqliteExportLogRepository.cpp -> persistence/src/repositories/SqliteExportLogRepository.cpp`
- `persistence/src/repositories/SqliteImportLogRepository.cpp -> persistence/src/repositories/SqliteImportLogRepository.cpp`
- `persistence/src/repositories/SqlitePropertyRepository.cpp -> persistence/src/repositories/SqlitePropertyRepository.cpp`
- `persistence/src/repositories/SqliteStatementDraftRepository.cpp -> persistence/src/repositories/SqliteStatementDraftRepository.cpp`
- `persistence/src/repositories/SqliteStatementRepository.cpp -> persistence/src/repositories/SqliteStatementRepository.cpp`
- `persistence/src/repositories/SqliteTransactionDraftRepository.cpp -> persistence/src/repositories/SqliteTransactionDraftRepository.cpp`
- `persistence/src/repositories/SqliteTransactionRepository.cpp -> persistence/src/repositories/SqliteTransactionRepository.cpp`

### External service adapters
- `services/poppler/src/PopplerAdapter.cpp -> services/poppler/src/PopplerAdapter.cpp`
- `services/poppler/src/PopplerEngine.cpp -> services/poppler/src/PopplerEngine.cpp`
- `services/poppler/src/PopplerService.cpp -> services/poppler/src/PopplerService.cpp`
- `services/opencv/src/OpenCvAdapter.cpp -> services/opencv/src/OpenCvAdapter.cpp`
- `services/opencv/src/OpenCvService.cpp -> services/opencv/src/OpenCvService.cpp`
- `services/opencv/src/CropEngine.cpp -> services/opencv/src/CropEngine.cpp`
- `services/opencv/src/DenoiseEngine.cpp -> services/opencv/src/DenoiseEngine.cpp`
- `services/opencv/src/DetectEngine.cpp -> services/opencv/src/DetectEngine.cpp`
- `services/opencv/src/MaskEngine.cpp -> services/opencv/src/MaskEngine.cpp`
- `services/tesseract/src/TesseractAdapter.cpp -> services/tesseract/src/TesseractAdapter.cpp`
- `services/tesseract/src/TesseractEngine.cpp -> services/tesseract/src/TesseractEngine.cpp`
- `services/tesseract/src/TesseractService.cpp -> services/tesseract/src/TesseractService.cpp`

### Debug adapters
- `debug/src/ErrorReporter.cpp -> debug/src/reporters/ErrorReporter.cpp`
- `debug/src/FileDebugger.cpp -> debug/src/runtime/FileDebugger.cpp`
- `debug/src/RunManager.cpp -> debug/src/runtime/RunManager.cpp`
- `debug/src/SpdlogDebugger.cpp -> debug/src/reporters/SpdlogDebugger.cpp`

### App composition root
- `app/src/main.cpp -> app/src/startup/main.cpp`
- `app/src/main_qml.cpp -> app/src/startup/main_qml.cpp`
- `app/src/Environment.cpp -> app/src/composition/Environment.cpp`
- `app/src/Environment.h -> app/include/app/composition/Environment.h`

---

## Consistent naming rules

Every application use case must use the same language:
- `<UseCase>.h`
- `<UseCase>Request.h`
- `<UseCase>Result.h`

Examples:
- `RunAnalysis.h`
- `RunAnalysisRequest.h`
- `RunAnalysisResult.h`

- `ImportStatement.h`
- `ImportStatementRequest.h`
- `ImportStatementResult.h`

- `ExportData.h`
- `ExportDataRequest.h`
- `ExportDataResult.h`

- `FinalizeDraft.h`
- `FinalizeDraftRequest.h`
- `FinalizeDraftResult.h`

- `SaveWorkspace.h`
- `SaveWorkspaceRequest.h`
- `SaveWorkspaceResult.h`

The current mixed naming is a problem and should be removed.

---

## Where controllers, presenters, and gateways belong

## Controllers
Controllers are outside core.

Current concrete place:
- `ui/include/ui/controllers/*`
- `ui/src/controllers/*`

These are inbound adapters.

They should:
- gather UI input
- build request objects
- call application use cases

They should not own business rules.

## Presenters
Presenter implementations are outside core.

If output boundary interfaces are needed, only the interfaces belong in core:
- `core/ports/presenters/*`

Concrete presenter implementations belong in:
- `ui/*`

In the current codebase, a lot of presenter-like work is spread across:
- `ui/state/*`
- `ui/analysis/*`
- `ui/import/*`
- `ui/export/*`
- `ui/payload/*`

## Gateways
Gateway interfaces belong in:
- `core/ports/repositories`
- `core/ports/storage`
- `core/ports/services`

Concrete gateway implementations belong outside core:
- `persistence/*`
- `services/*`

---

## How ui / persistence / debug / services / api attach to ports

## UI
UI consumes `core/application` use cases and optionally `core/ports/presenters`.

UI must not implement business rules.

Concrete current attachment points that stay in UI:
- `ui/controllers/*`
- `ui/state/*`
- `ui/models/*`
- `ui/analysis/*`
- `ui/import/*`
- `ui/export/*`

## Persistence
Persistence implements:
- `core/ports/repositories/*`
- `core/ports/storage/*`

Examples from current codebase:
- `persistence/src/repositories/SqliteActorRepository.cpp`
- `persistence/src/AppStateStore.cpp`
- `persistence/src/SqliteRegistry.cpp`

## Services
Services implement:
- `core/ports/services/*`

Examples:
- `services/poppler/src/PopplerService.cpp`
- `services/opencv/src/OpenCvService.cpp`
- `services/tesseract/src/TesseractService.cpp`

## API
Current `api` is effectively a ports package plus technology DTOs.

Long-term target:
- pure service interfaces move into `core/ports/services`
- technology-specific request/response types stay where they are only if you explicitly keep a technical anti-corruption layer

## Debug
`debug` stays outside core.

It may plug into:
- `core/errors/IErrorReporter`
- logging or tracing infrastructure

It is tooling / adapter code, not enterprise or application business rules.

## App
`app` remains the composition root.

Concrete current entry points:
- `app/src/main.cpp`
- `app/src/main_qml.cpp`
- `app/src/Environment.cpp`

This layer wires:
- UI
- persistence
- services
- debug
- application services

---

## Namespace target

- `core::domain` for domain entities and value objects
- `core::application` for use cases and application workflow state
- `core::ports::repositories` for repository ports
- `core::ports::storage` for storage ports
- `core::ports::services` for external service ports
- `core::ports::presenters` for output boundary interfaces

Namespace changes are expected as part of the refactor.

---

## GPT refactor workflow

When a GPT agent edits this project, it should use this order:
1. Move headers to their target folders.
2. Move `.cpp` files to their target folders.
3. Update include paths.
4. Update namespaces.
5. Update forward declarations.
6. Update CMakeLists and project references.
7. Build.
8. Record progress in the section below.

---

## Progress tracking

Instruction for GPT agents:
- When you complete a move or rename, replace `None yet.` with concrete bullet points.
- Record exact files, not vague summaries.
- Keep this section append-only except when moving an item from `In progress` to `Completed`.

### Completed
- None yet.

### In progress
- None yet.

### Open
- Split `core/models` into `domain` and `valueobjects`.
- Move workspace state out of `core/models`.
- Create `core/ports` and relocate interfaces.
- Normalize use-case naming to `UseCase / Request / Result`.
- Reattach adapters to the new ports.
- Update namespaces.
- Update includes and build files.

---

## Final summary

The target is a strict clean-architecture layout:
- `core/models` for enterprise business rules only
- `core/application` for application business rules only
- `core/ports` for all interfaces used by the application layer
- `ui`, `persistence`, `services`, `debug` as adapters
- `app` as composition root and framework shell

This removes the current confusion between:
- domain models vs workflow state
- ports vs implementations
- use cases vs adapters
- controller/presenter/gateway roles
