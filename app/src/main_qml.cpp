/**
 * @file app/src/main_qml.cpp
 * @brief Boots the QML application shell and wires UI-facing services.
 */

#ifdef USE_QML
#include "MainWindow.h"
#include "ui/bootstrap/AppContext.h"
#include "api/opencv/IOpenCvAdapter.h"
#include "api/opencv/IOpenCvService.h"
#include "api/poppler/IPopplerAdapter.h"
#include "api/poppler/IPopplerService.h"
#include "api/tesseract/ITesseractAdapter.h"
#include "api/tesseract/ITesseractService.h"
#include "core/application/AnalysisService.h"
#include "core/application/AppStateFacade.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/export/ExportService.h"
#include "core/export/ExportTypes.h"
#include "core/import/IImportStatement.h"
#include "core/jobs/JobSystem.h"
#include "core/models/AppState.h"
#include "core/models/DeletionImpact.h"
#include "debug/DebugDefaults.h"

using core::domain::AppState;
using core::domain::DeletionImpact;
#include "debug/ErrorReporter.h"
#include "debug/FileDebugger.h"
#include "ui/observability/ErrorCodes.h"
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
#include "ui/controllers/StatementController.h"
#include "ui/controllers/StorageController.h"
#include "ui/controllers/TransactionController.h"
#include "ui/export/AppStateSnapshot.h"
#include "ui/export/ExportRunner.h"
#include "ui/import/ImportJobBridge.h"
#include "ui/observability/Origins.h"
#include "ui/observability/Trace.h"
#include "ui/state/AppStateClone.h"
#include "ui/state/StateFacade.h"
#include <QApplication>
#include <QList>
#include <QQmlEngine>
#include <QQmlError>

#include <exception>
#include <memory>
#include <utility>

std::shared_ptr<api::poppler::IPopplerAdapter>
createPopplerAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<api::opencv::IOpenCvAdapter>
createOpenCvAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<api::tesseract::ITesseractAdapter>
createTesseractAdapter(std::shared_ptr<IDebugger> dbg);

namespace api {
namespace poppler {
std::shared_ptr<IPopplerService>
createPopplerService(std::shared_ptr<IPopplerAdapter> adapter);
}
} // namespace api
namespace api {
namespace opencv {
std::shared_ptr<IOpenCvService>
createOpenCvService(std::shared_ptr<IOpenCvAdapter> adapter);
}
} // namespace api
namespace api {
namespace tesseract {
std::shared_ptr<ITesseractService>
createTesseractService(std::shared_ptr<ITesseractAdapter> adapter);
}
} // namespace api

namespace {

core::exporting::ExportFormat
toCoreExportFormat(ui::qml::contracts::ExportFormat format) {
  switch (format) {
  case ui::qml::contracts::ExportFormat::Csv:
    return core::exporting::ExportFormat::Csv;
  case ui::qml::contracts::ExportFormat::Xlsx:
    return core::exporting::ExportFormat::Xlsx;
  default:
    ui::observability::reportFlow(
        core::errors::ErrorSeverity::Warning,
        ui::observability::codes::FlowExportFallback,
        ui::observability::origins::app::kToCoreExportFormat,
        "Unknown export format, fallback to CSV",
        {{ui::observability::context::kFormat,
          std::to_string(static_cast<int>(format))}});
    return core::exporting::ExportFormat::Csv;
  }
}

ui::exporting::ExportResult
executeExport(std::shared_ptr<const AppState> snapshot,
              const ui::exporting::ExportRequest &request) {
  core::exporting::ExportRequest exportRequest;
  exportRequest.outputPath = request.path.toStdString();
  exportRequest.includeFormulas = request.includeFormulas;
  exportRequest.locale = request.locale.toStdString();
  exportRequest.stateSnapshot = std::move(snapshot);
  exportRequest.format = toCoreExportFormat(request.format);

  core::exporting::ExportService exporter;
  const auto result = exporter.exportData(exportRequest);

  return {result.status == core::exporting::ExportStatus::Ok,
          QString::fromStdString(result.errorCode),
          QString::fromStdString(result.message)};
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
  std::shared_ptr<core::application::AnalysisService> analysisService;
};

UiControllers setupUiControllers(
    QApplication &app, MainWindow &w,
    core::application::AppStateFacade &appStateFacade,
    const std::shared_ptr<core::errors::IErrorReporter> &errorReporter) {
  UiControllers ui;

  ui.storage = new ui::StorageController(&appStateFacade, &w);
  w.setQmlContextProperty(ui::qml::contracts::context::kStorageController,
                          ui.storage);
  if (auto *appContext = w.appContext())
    appContext->setStorageController(ui.storage);

  const auto exportSnapshotProvider = [&appStateFacade]() {
    return ui::exporting::createSnapshot(appStateFacade.state());
  };

  ui.analysisService = std::make_shared<core::application::AnalysisService>();
  ui.annual = new ui::AnnualController(&appStateFacade, &w);
  ui.actor = new ui::ActorController(&appStateFacade, &w);
  ui.property = new ui::PropertyController(&appStateFacade, &w);
  ui.contract = new ui::ContractController(&appStateFacade, &w);
  ui.statement = new ui::StatementController(&appStateFacade, &w);
  ui.transaction = new ui::TransactionController(&appStateFacade, &w);
  ui.draft = new ui::DraftController(&appStateFacade, &w);
  ui.analysisController = new ui::AnalysisController(
      &appStateFacade, exportSnapshotProvider, ui.analysisService, &w);
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
  ui.exportCtrl =
      new ui::ExportController(exportSnapshotProvider, exportRunner, &w);
  w.setQmlContextProperty(ui::qml::contracts::context::kExportController,
                          ui.exportCtrl);
  if (auto *appContext = w.appContext())
    appContext->setExportController(ui.exportCtrl);

  ui.language = new ui::LanguageController(&app, w.qmlEngine(), &w);
  w.setQmlContextProperty(ui::qml::contracts::context::kLanguageController,
                          ui.language);
  if (auto *appContext = w.appContext())
    appContext->setLanguageController(ui.language);

  auto importJobSystemFactory = [dbg = std::make_shared<FileDebugger>(
                                   "", std::string(debug::defaults::kImportProcessName)),
                                 &errorReporter]() {
    auto popplerAdapter = createPopplerAdapter(dbg);
    auto opencvAdapter = createOpenCvAdapter(dbg);
    auto tesseractAdapter = createTesseractAdapter(dbg);

    auto poppler = api::poppler::createPopplerService(popplerAdapter);
    auto opencv = api::opencv::createOpenCvService(opencvAdapter);
    auto tesseract = api::tesseract::createTesseractService(tesseractAdapter);

    auto importSvc = core::importing::createImportStatement(poppler, opencv, tesseract, errorReporter);
    return std::make_shared<core::jobs::JobSystem>(importSvc);
  };

  const auto importSnapshotProvider = [&appStateFacade, &w]() {
    const auto liveState = appStateFacade.state();
    if (auto *session = w.dataSession()) {
      AppState snapshot;
      snapshot.actors = ui::cloneStateItems(session->actors()->actors());
      snapshot.properties = ui::cloneStateItems(session->properties()->properties());
      snapshot.contracts = ui::cloneStateItems(session->contracts()->contracts());
      snapshot.statements = ui::cloneStateItems(session->statements()->statements());
      snapshot.transactions = ui::cloneStateItems(session->transactions()->transactions());
      snapshot.analyses = ui::cloneStateItems(session->analyses()->analyses());
      snapshot.annuals = ui::cloneStateItems(session->annuals()->annuals());
      if (snapshot.actors.empty()) snapshot.actors = liveState.actors;
      if (snapshot.properties.empty()) snapshot.properties = liveState.properties;
      if (snapshot.contracts.empty()) snapshot.contracts = liveState.contracts;
      if (snapshot.statements.empty()) snapshot.statements = liveState.statements;
      if (snapshot.transactions.empty()) snapshot.transactions = liveState.transactions;
      if (snapshot.analyses.empty()) snapshot.analyses = liveState.analyses;
      if (snapshot.annuals.empty()) snapshot.annuals = liveState.annuals;
      return snapshot;
    }
    return liveState;
  };

  ui.import = new ui::ImportController(importJobSystemFactory, errorReporter, &w);
  ui.import->setStateSnapshotProvider(importSnapshotProvider);
  w.setQmlContextProperty(ui::qml::contracts::context::kImportController,
                          ui.import);
  if (auto *appContext = w.appContext())
    appContext->setImportController(ui.import);

  w.addImageProvider(ui::qml::contracts::providers::kImportProof,
                     ui::importing::createDraftProofProvider(ui.import));

  return ui;
}

void wireAppStateToSession(
    MainWindow &w, core::application::AppStateFacade &appStateFacade,
    const std::shared_ptr<core::errors::IErrorReporter> &errorReporter) {
  if (w.dataSession()) {
    w.dataSession()->loadFromState(appStateFacade.state());
  }

  appStateFacade.setStateChangedCallback([&](const AppState &st) {
    if (w.dataSession()) {
      w.dataSession()->loadFromState(st);
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
  QObject::connect(&w, &MainWindow::saveFileAsRequested, storage,
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
int startQmlApp(QApplication &app, core::application::AppStateFacade &appStateFacade) {
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
