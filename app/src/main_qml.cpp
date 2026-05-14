/**
 * @file app/src/main_qml.cpp
 * @brief Boots the QML application shell and wires UI-facing services.
 */

#ifdef USE_QML
#include "MainWindow.h"
#include "core/application/analysis/AnalysisService.h"
#include "core/application/export/ExportLog.h"
#include "core/application/export/ExportRequest.h"
#include "core/application/export/ExportResult.h"
#include "core/application/export/ExportService.h"
#include "core/application/import/IImportStatement.h"
#include "core/application/import/draft/DraftMatcher.h"
#include "core/application/import/draft/IImportMatcherService.h"
#include "core/application/storage/DeletionImpact.h"
#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/application/workspace/WorkspaceFacade.h"
#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/jobs/JobSystem.h"
#include "core/ports/archive/IArchive.h"
#include "core/ports/image-processing/IImageProcessor.h"
#include "core/ports/pdf-rendering/IPdfRenderer.h"
#include "core/ports/analysis-image-renderer/IAnalysisImageRenderer.h"
#include "core/ports/xlsx-writer/IXlsxWriter.h"
#include "core/ports/presenters/IAnalysisPresenter.h"
#include "core/ports/presenters/IExportPresenter.h"
#include "core/ports/presenters/IImportPresenter.h"
#include "core/ports/presenters/IWorkspacePresenter.h"
#include "core/ports/text-recognition/ITextRecognizer.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "debug/DebugDefaults.h"
#include "analysis-image-renderer/OpenCvAnalysisImageRendererAdapter.h"
#include "xlsx-writer/XlntTableWriterAdapter.h"
#include "ui/bootstrap/AppContext.h"

using core::domain::DeletionImpact;
#include "archive/ZipArchiveAdapter.h"
#include "debug/ErrorReporter.h"
#include "debug/FileDebugger.h"
#include "ui/bootstrap/QmlContracts.h"
#include "ui/controllers/ActorController.h"
#include "ui/controllers/AnalysisController.h"
#include "ui/controllers/AnnualController.h"
#include "ui/controllers/ContractController.h"
#include "ui/controllers/DraftController.h"
#include "ui/controllers/ExportController.h"
#include "ui/controllers/ImportController.h"
#include "ui/controllers/LanguageController.h"
#include "ui/controllers/PropertyController.h"
#include "ui/controllers/SettingsController.h"
#include "ui/controllers/StatementController.h"
#include "ui/controllers/StorageController.h"
#include "ui/controllers/TransactionController.h"
#include "ui/export/ExportRunner.h"
#include "ui/export/WorkspaceSnapshot.h"
#include "ui/observability/ErrorCodes.h"
#include "ui/observability/Origins.h"
#include "ui/observability/Trace.h"
#include "ui/state/StateFacade.h"
#include "ui/state/WorkspaceClone.h"
#include <QApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QQmlEngine>
#include <QQmlError>

#include <exception>
#include <memory>
#include <utility>

std::shared_ptr<core::ports::pdf_rendering::IPdfRenderer>
createPdfRendererAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<core::ports::image_processing::IImageProcessor>
createImageProcessorAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<core::ports::text_recognition::ITextRecognizer>
createTextRecognizerAdapter(std::shared_ptr<IDebugger> dbg);

namespace {

core::ports::workspace::StatementDraftSnapshot toStatementDraftSnapshot(
    const core::application::importing::draft::StatementDraft &draft) {
  core::ports::workspace::StatementDraftSnapshot snapshot;
  snapshot.id = draft.id;
  snapshot.name = draft.name;
  snapshot.transactionIds = draft.transactionIds;
  snapshot.createdAt = draft.createdAt;
  snapshot.updatedAt = draft.updatedAt;
  snapshot.transactions.reserve(draft.transactions.size());
  for (const auto &tx : draft.transactions) {
    core::ports::workspace::TransactionDraftSnapshot txSnapshot;
    txSnapshot.id = tx.id;
    txSnapshot.statementDraftId = tx.statementDraftId;
    txSnapshot.name = tx.name;
    txSnapshot.bookingDate = tx.bookingDate;
    txSnapshot.valuta = tx.valuta;
    txSnapshot.amount = tx.amount;
    txSnapshot.actorId = tx.actorId;
    txSnapshot.contractId = tx.contractId;
    txSnapshot.propertyIds = tx.propertyIds;
    txSnapshot.status = static_cast<int>(tx.status);
    txSnapshot.allocatable = tx.allocatable;
    txSnapshot.position = tx.position;
    txSnapshot.metadata = tx.metadata;
    snapshot.transactions.push_back(std::move(txSnapshot));
  }
  return snapshot;
}

core::ports::workspace::ImportLogsCommand toImportLogsCommand(
    const std::vector<core::application::importing::ImportLog> &logs) {
  core::ports::workspace::ImportLogsCommand command;
  command.logs.reserve(logs.size());
  for (const auto &log : logs) {
    command.logs.push_back({log.id,       log.time,
                            log.type,     log.file,
                            log.status,   log.message,
                            log.draftAttached,
                            log.draftId,  log.statementDraftIds,
                            log.statementId});
  }
  return command;
}

core::ports::workspace::ExportLogsCommand toExportLogsCommand(
    const std::vector<core::application::exporting::ExportLog> &logs) {
  core::ports::workspace::ExportLogsCommand command;
  command.logs.reserve(logs.size());
  for (const auto &log : logs) {
    command.logs.push_back({log.id,      log.time,
                            log.targetPath,
                            log.status,  log.message,
                            log.payload, log.annualIds,
                            log.analysisIds});
  }
  return command;
}

core::application::exporting::ExportFormat
toCoreExportFormat(ui::qml::contracts::ExportFormat format) {
  switch (format) {
  case ui::qml::contracts::ExportFormat::Csv:
    return core::application::exporting::ExportFormat::Csv;
  case ui::qml::contracts::ExportFormat::Xlsx:
    return core::application::exporting::ExportFormat::Xlsx;
  default:
    ui::observability::reportFlow(
        core::errors::ErrorSeverity::Warning,
        ui::observability::codes::FlowExportFallback,
        ui::observability::origins::app::kToCoreExportFormat,
        "Unknown export format, fallback to CSV",
        {{ui::observability::context::kFormat,
          std::to_string(static_cast<int>(format))}});
    return core::application::exporting::ExportFormat::Csv;
  }
}

ui::exporting::ExportResult
executeExport(std::shared_ptr<const core::domain::catalog::WorkspaceCatalog> snapshot,
              const ui::exporting::ExportRequest &request) {
  std::shared_ptr<core::ports::archive::IArchive> archiveAdapter;
  std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> xlsxWriter =
      std::make_shared<infra::xlsx_writer::XlntTableWriterAdapter>();
  std::shared_ptr<core::ports::analysis_image_renderer::IAnalysisImageRenderer> imageRenderer =
      std::make_shared<infra::analysis_image_renderer::OpenCvAnalysisImageRendererAdapter>();
  core::application::exporting::ExportRequest exportRequest;
  exportRequest.outputPath = request.path.toStdString();
  exportRequest.includeFormulas = request.includeFormulas;
  exportRequest.locale = request.locale.toStdString();
  exportRequest.stateSnapshot = std::move(snapshot);
  exportRequest.format = toCoreExportFormat(request.format);

  QJsonParseError parseError;
  const QJsonDocument payloadDoc =
      QJsonDocument::fromJson(request.payload.toUtf8(), &parseError);
  if (parseError.error == QJsonParseError::NoError && payloadDoc.isObject()) {
    const QJsonObject payloadObj = payloadDoc.object();
    const int packageIndex =
        payloadObj.value(QStringLiteral("packageFormatIndex")).toInt(0);
    switch (packageIndex) {
    case 1:
      exportRequest.packageFormat =
          core::application::exporting::PackageFormat::Zip;
      archiveAdapter = std::make_shared<infra::archive::ZipArchiveAdapter>();
      break;
    default:
      exportRequest.packageFormat =
          core::application::exporting::PackageFormat::None;
      break;
    }

    const QJsonArray items =
        payloadObj.value(QStringLiteral("items")).toArray();
    for (const QJsonValue &value : items) {
      if (!value.isObject())
        continue;
      const QJsonObject item = value.toObject();
      if (item.value(QStringLiteral("objectType"))
              .toString()
              .compare(QStringLiteral("Analysis"), Qt::CaseInsensitive) != 0)
        continue;

      const QString exportType =
          item.value(QStringLiteral("exportType")).toString();
      core::application::exporting::AnalysisExportItem analysisItem;
      analysisItem.annualId =
          item.value(QStringLiteral("annualId")).toString().toStdString();
      analysisItem.analysisId =
          item.value(QStringLiteral("objectId")).toString().toStdString();
      analysisItem.name =
          item.value(QStringLiteral("objectName")).toString().toStdString();

      const QString normalized = exportType.trimmed().toLower();
      if (normalized == QStringLiteral("xlsx"))
        analysisItem.format =
            core::application::exporting::AnalysisExportFormat::Xlsx;
      else if (normalized == QStringLiteral("jpg") ||
               normalized == QStringLiteral("jpeg"))
        analysisItem.format =
            core::application::exporting::AnalysisExportFormat::Jpg;
      else if (normalized == QStringLiteral("png"))
        analysisItem.format =
            core::application::exporting::AnalysisExportFormat::Png;
      else
        analysisItem.format =
            core::application::exporting::AnalysisExportFormat::Csv;

      if (!analysisItem.analysisId.empty()) {
        exportRequest.analysisItems.push_back(std::move(analysisItem));
      }
    }
  }

  core::application::exporting::ExportService exporter(
      std::move(archiveAdapter), std::move(xlsxWriter), std::move(imageRenderer));
  const auto result = exporter.exportData(exportRequest);
  return result;
}

struct UiControllers {
  ui::StorageController *storage = nullptr;
  ui::AnnualController *annual = nullptr;
  ui::ActorController *actor = nullptr;
  ui::PropertyController *property = nullptr;
  ui::ContractController *contract = nullptr;
  ui::StatementController *statement = nullptr;
  ui::TransactionController *transaction = nullptr;
  ui::DraftController *draft = nullptr;
  ui::AnalysisController *analysisController = nullptr;
  ui::ExportController *exportCtrl = nullptr;
  ui::ImportController *import = nullptr;
  ui::LanguageController *language = nullptr;
  std::shared_ptr<core::application::analysis::AnalysisService> analysisService;
};

struct WorkspacePresenterAdapter final
    : core::ports::presenters::IWorkspacePresenter {
  core::ports::presenters::WorkspacePresentation
  present(const core::ports::presenters::WorkspacePresentation &result)
      const override {
    auto out = result;
    if (!out.hasCurrentPath) {
      out.currentPath.clear();
      return out;
    }
    out.hasCurrentPath = !out.currentPath.empty();
    return out;
  }
};

struct AnalysisPresenterAdapter final
    : core::ports::presenters::IAnalysisPresenter {
  core::ports::presenters::AnalysisPresentation
  present(const core::ports::presenters::AnalysisPresentation &result)
      const override {
    return result;
  }
};

struct ExportPresenterAdapter final
    : core::ports::presenters::IExportPresenter {
  core::ports::presenters::ExportPresentation
  present(const core::ports::presenters::ExportPresentation &result)
      const override {
    return result;
  }
};

struct ImportPresenterAdapter final
    : core::ports::presenters::IImportPresenter {
  core::ports::presenters::ImportPresentation
  present(const core::ports::presenters::ImportPresentation &result)
      const override {
    return result;
  }
};

struct ImportMatcherServiceAdapter final
    : core::application::importing::draft::IImportMatcherService {
  core::application::importing::draft::ImportMatcherPresentation
  buildImportSuggestions(
      const core::domain::catalog::WorkspaceCatalog &state,
      const core::application::importing::draft::TransactionDraft &transaction)
      const override {
    return core::application::importing::draft::buildImportSuggestions(
        state, transaction);
  }

  core::application::importing::draft::DraftTextSignals buildDraftTextSignals(
      const core::domain::catalog::WorkspaceCatalog &state,
      const core::application::importing::draft::TransactionDraft &transaction)
      const override {
    return core::application::importing::draft::buildDraftTextSignals(
        state, transaction);
  }

  core::application::importing::draft::DraftDerivedState buildDraftDerivedState(
      const core::domain::catalog::WorkspaceCatalog &state,
      const core::application::importing::draft::DraftLinkSelection &selection)
      const override {
    return core::application::importing::draft::buildDraftDerivedState(
        state, selection);
  }

  std::string resolveActorId(const core::domain::catalog::WorkspaceCatalog &state,
                             const std::string &text) const override {
    return core::application::importing::draft::resolveActorId(state, text);
  }

  std::string resolveContractId(const core::domain::catalog::WorkspaceCatalog &state,
                                const std::string &text) const override {
    return core::application::importing::draft::resolveContractId(state, text);
  }

  bool
  contractIsFullyAllocatable(const core::domain::catalog::WorkspaceCatalog &state,
                             const std::string &contractId) const override {
    return core::application::importing::draft::contractIsFullyAllocatable(
        state, contractId);
  }

  core::domain::catalog::WorkspaceCatalog withFallbackState(
      core::domain::catalog::WorkspaceCatalog primary,
      const core::domain::catalog::WorkspaceCatalog &fallback) const override {
    return core::application::importing::draft::withFallbackState(
        std::move(primary), fallback);
  }

  std::vector<std::string>
  referenceAliasesFromMetadata(const std::string &metadata) const override {
    return core::application::importing::draft::referenceAliasesFromMetadata(
        metadata);
  }
};

UiControllers setupUiControllers(
    QApplication &app, MainWindow &w,
    core::application::WorkspaceFacade &appStateFacade,
    const std::shared_ptr<core::errors::IErrorReporter> &errorReporter) {
  UiControllers ui;

  auto workspacePresenter = std::make_shared<WorkspacePresenterAdapter>();
  ui.storage =
      new ui::StorageController(&appStateFacade, workspacePresenter, &w);
  w.setQmlContextProperty(ui::qml::contracts::context::kStorageController,
                          ui.storage);
  if (auto *appContext = w.appContext())
    appContext->setStorageController(ui.storage);

  const auto analysisSnapshotProvider = [&appStateFacade]() {
    return std::make_shared<const core::domain::catalog::WorkspaceCatalog>(appStateFacade.catalogState());
  };

  const auto exportSnapshotProvider = [&appStateFacade]() {
    return std::make_shared<const core::application::workspace::WorkspaceSessionState>(appStateFacade.state());
  };

  ui.analysisService =
      std::make_shared<core::application::analysis::AnalysisService>();
  auto analysisPresenter = std::make_shared<AnalysisPresenterAdapter>();
  ui.annual = new ui::AnnualController(&appStateFacade, &w);
  ui.actor = new ui::ActorController(&appStateFacade, &w);
  ui.property = new ui::PropertyController(&appStateFacade, &w);
  ui.contract = new ui::ContractController(&appStateFacade, &w);
  ui.statement = new ui::StatementController(&appStateFacade, &w);
  ui.transaction = new ui::TransactionController(&appStateFacade, &w);
  auto importMatcherService = std::make_shared<ImportMatcherServiceAdapter>();
  ui.draft = new ui::DraftController(&appStateFacade, importMatcherService, &w);
   ui.analysisController =
       new ui::AnalysisController(&appStateFacade, analysisSnapshotProvider,
                                  ui.analysisService, analysisPresenter, &w);
  w.setQmlContextProperty(ui::qml::contracts::context::kAnnualController,
                          ui.annual);
  if (auto *appContext = w.appContext())
    appContext->setAnnualController(ui.annual);
  w.setQmlContextProperty(ui::qml::contracts::context::kActorController,
                          ui.actor);
  if (auto *appContext = w.appContext())
    appContext->setActorController(ui.actor);
  w.setQmlContextProperty(ui::qml::contracts::context::kPropertyController,
                          ui.property);
  if (auto *appContext = w.appContext())
    appContext->setPropertyController(ui.property);
  w.setQmlContextProperty(ui::qml::contracts::context::kContractController,
                          ui.contract);
  if (auto *appContext = w.appContext())
    appContext->setContractController(ui.contract);
  w.setQmlContextProperty(ui::qml::contracts::context::kStatementController,
                          ui.statement);
  if (auto *appContext = w.appContext())
    appContext->setStatementController(ui.statement);
  w.setQmlContextProperty(ui::qml::contracts::context::kTransactionController,
                          ui.transaction);
  if (auto *appContext = w.appContext())
    appContext->setTransactionController(ui.transaction);
  w.setQmlContextProperty(ui::qml::contracts::context::kDraftController,
                          ui.draft);
  if (auto *appContext = w.appContext())
    appContext->setDraftController(ui.draft);
  w.setQmlContextProperty(ui::qml::contracts::context::kAnalysisController,
                          ui.analysisController);
  if (auto *appContext = w.appContext())
    appContext->setAnalysisController(ui.analysisController);

  auto exportRunner =
      std::make_shared<ui::exporting::ExportRunner>(executeExport);
  auto exportPresenter = std::make_shared<ExportPresenterAdapter>();
  ui.exportCtrl = new ui::ExportController(exportSnapshotProvider, exportRunner,
                                           exportPresenter, &w);
  ui.exportCtrl->setExportLogsStore(
      [&appStateFacade](
          const std::vector<core::application::exporting::ExportLog> &logs) {
        appStateFacade.setExportLogs(toExportLogsCommand(logs));
      });
  w.setQmlContextProperty(ui::qml::contracts::context::kExportController,
                          ui.exportCtrl);
  if (auto *appContext = w.appContext())
    appContext->setExportController(ui.exportCtrl);

  ui.language = new ui::LanguageController(&app, w.qmlEngine(), &w);
  w.setQmlContextProperty(ui::qml::contracts::context::kLanguageController,
                          ui.language);
  if (auto *appContext = w.appContext())
    appContext->setLanguageController(ui.language);

  auto *settingsController = new ui::SettingsController(&w);
  w.setQmlContextProperty(ui::qml::contracts::context::kSettingsController,
                          settingsController);
  if (auto *appContext = w.appContext())
    appContext->setSettingsController(settingsController);

  auto importJobSystemFactory =
      [dbg = std::make_shared<FileDebugger>(
           "", std::string(debug::defaults::kImportProcessName)),
       &errorReporter]() {
        auto popplerAdapter = createPdfRendererAdapter(dbg);
        auto opencvAdapter = createImageProcessorAdapter(dbg);
        auto tesseractAdapter = createTextRecognizerAdapter(dbg);

        auto poppler = popplerAdapter;
        auto opencv = opencvAdapter;
        auto tesseract = tesseractAdapter;

        auto importSvc = core::application::importing::createImportStatement(
            poppler, opencv, tesseract, errorReporter);
        return std::make_shared<core::jobs::JobSystem>(importSvc);
      };

  auto importPresenter = std::make_shared<ImportPresenterAdapter>();

  const auto importSnapshotProvider = [&appStateFacade, &w]() {
    const auto liveState = appStateFacade.state();
    if (auto *session = w.dataSession()) {
      core::application::workspace::WorkspaceSessionState snapshot;
      snapshot.catalog.setActors(
          ui::cloneStateItems(session->actors()->actors()));
      snapshot.catalog.setProperties(
          ui::cloneStateItems(session->properties()->properties()));
      snapshot.catalog.setContracts(
          ui::cloneStateItems(session->contracts()->contracts()));
      snapshot.catalog.setStatements(
          ui::cloneStateItems(session->statements()->statements()));
      snapshot.catalog.setTransactions(
          ui::cloneStateItems(session->transactions()->transactions()));
      snapshot.catalog.setAnalyses(
          ui::cloneStateItems(session->analyses()->analyses()));
      snapshot.catalog.setAnnuals(
          ui::cloneStateItems(session->annuals()->annuals()));
      if (snapshot.catalog.actors().empty())
        snapshot.catalog.setActors(liveState.catalog.actors());
      if (snapshot.catalog.properties().empty())
        snapshot.catalog.setProperties(liveState.catalog.properties());
      if (snapshot.catalog.contracts().empty())
        snapshot.catalog.setContracts(liveState.catalog.contracts());
      if (snapshot.catalog.statements().empty())
        snapshot.catalog.setStatements(liveState.catalog.statements());
      if (snapshot.catalog.transactions().empty())
        snapshot.catalog.setTransactions(liveState.catalog.transactions());
      if (snapshot.catalog.analyses().empty())
        snapshot.catalog.setAnalyses(liveState.catalog.analyses());
      if (snapshot.catalog.annuals().empty())
        snapshot.catalog.setAnnuals(liveState.catalog.annuals());
      snapshot.workflow = liveState.workflow;
      return snapshot;
    }
    return liveState;
  };

  ui.import =
      new ui::ImportController(importJobSystemFactory, errorReporter,
                               importPresenter, importMatcherService, &w);
  ui.import->setStateSnapshotProvider(importSnapshotProvider);
  ui.import->setImportLogsStore(
      [&appStateFacade](
          const std::vector<core::application::importing::ImportLog> &logs) {
        appStateFacade.setImportLogs(toImportLogsCommand(logs));
      });
  ui.import->setStatementDraftStore(
      [&appStateFacade](
          const core::application::importing::draft::StatementDraft &draft) {
        appStateFacade.saveStatementDraft({toStatementDraftSnapshot(draft)});
      });
  w.setQmlContextProperty(ui::qml::contracts::context::kImportController,
                          ui.import);
  if (auto *appContext = w.appContext())
    appContext->setImportController(ui.import);

  return ui;
}

void wireAppStateToSession(
    MainWindow &w, core::application::WorkspaceFacade &appStateFacade,
    const std::shared_ptr<core::errors::IErrorReporter> &errorReporter) {
  if (w.dataSession()) {
    w.dataSession()->loadFromState(appStateFacade.catalogState());
  }

  appStateFacade.setStateChangedCallback(
      [&](const core::application::workspace::WorkspaceSessionState &document) {
        if (w.dataSession()) {
          w.dataSession()->loadFromState(document.catalog);
        }
      });

  appStateFacade.setDeletionImpactCallback([&](const DeletionImpact &impact) {
    try {
      if (w.dataSession())
        w.dataSession()->applyDeletionImpact(impact);
    } catch (...) {
      if (errorReporter)
        errorReporter->reportException(
            core::errors::ErrorSeverity::Error,
            "app::wireAppStateToSession::applyDeletionImpact",
            std::current_exception());
    }
  });
}

void wireFileSignals(MainWindow &w, ui::StorageController *storage) {
  if (!storage)
    return;

  QObject::connect(&w, &MainWindow::newFileRequested, storage,
                   [storage](const QString &path) { storage->newFile(path); });
  QObject::connect(&w, &MainWindow::openFileRequested, storage,
                   [storage](const QString &path) { storage->openFile(path); });
  QObject::connect(&w, &MainWindow::saveFileRequested, storage,
                   [storage]() { storage->saveFile(); });
  QObject::connect(
      &w, &MainWindow::saveFileAsRequested, storage,
      [storage](const QString &path) { storage->saveFileAs(path); });
  QObject::connect(storage, &ui::StorageController::operationSucceeded, &w,
                   &MainWindow::handleStorageOperationSucceeded);
  QObject::connect(storage, &ui::StorageController::operationFailed, &w,
                   &MainWindow::handleStorageOperationFailed);
}

void wireQmlWarnings(
    MainWindow &w,
    const std::shared_ptr<core::errors::IErrorReporter> &errorReporter) {
  auto *engine = w.qmlEngine();
  if (!engine || !errorReporter)
    return;

  QObject::connect(
      engine, &QQmlEngine::warnings, &w,
      [errorReporter](const QList<QQmlError> &warnings) {
        for (const auto &warning : warnings) {
          core::errors::ErrorEvent event;
          event.severity = core::errors::ErrorSeverity::Warning;
          event.code = ui::observability::codes::QmlWarning;
          event.origin = ui::observability::origins::app::kQmlWarnings;
          event.message = warning.description().toStdString();
          event.context.emplace_back(ui::observability::context::kUrl,
                                     warning.url().toString().toStdString());
          event.context.emplace_back(ui::observability::context::kLine,
                                     std::to_string(warning.line()));
          event.context.emplace_back(ui::observability::context::kColumn,
                                     std::to_string(warning.column()));
          errorReporter->report(event);
        }
      });
}

} // namespace

/**
 * @brief Initialize and run the QML-based UI.
 * @param app Reference to the already-created QApplication instance.
 * @param appStateFacade Reference to the application facade that manages
 * application state.
 * @return Return value from `QApplication::exec()`.
 */
int startQmlApp(QApplication &app,
                core::application::WorkspaceFacade &appStateFacade) {
  MainWindow w;

  auto errorReporter = core::errors::globalErrorReporter();
  if (!errorReporter) {
    errorReporter = debug::createDefaultErrorReporter();
    core::errors::setGlobalErrorReporter(errorReporter);
  }
  appStateFacade.setErrorReporter(errorReporter);

  const UiControllers ui =
      setupUiControllers(app, w, appStateFacade, errorReporter);

  wireAppStateToSession(w, appStateFacade, errorReporter);

  wireFileSignals(w, ui.storage);

  wireQmlWarnings(w, errorReporter);

  // Load QML after all context properties/providers are installed.
  w.loadQml();

  w.show();
  const int exitCode = app.exec();

  appStateFacade.setStateChangedCallback({});
  appStateFacade.setDeletionImpactCallback({});
  appStateFacade.setErrorReporter({});

  return exitCode;
}

#endif
