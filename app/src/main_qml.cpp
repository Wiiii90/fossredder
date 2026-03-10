/**
 * @file main_qml.cpp
 * @brief QML startup implementation.
 *
 * This translation unit implements `startQmlApp` which initializes the main
 * window, registers UI controllers and services, connects signals/slots and
 * starts the Qt event loop.
 */

#ifdef USE_QML
#include "MainWindow.h"
#include "api/opencv/IOpenCvAdapter.h"
#include "api/opencv/IOpenCvService.h"
#include "api/poppler/IPopplerAdapter.h"
#include "api/poppler/IPopplerService.h"
#include "api/tesseract/ITesseractAdapter.h"
#include "api/tesseract/ITesseractService.h"
#include "core/analysis/AnalysisEngine.h"
#include "core/controllers/CsvController.h"
#include "core/controllers/ExportController.h"
#include "core/controllers/ImportController.h"
#include "core/controllers/StatementController.h"
#include "core/controllers/XlsxController.h"
#include "core/errors/DebuggerErrorReporter.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/export/ExportOptions.h"
#include "core/import/IImportStatement.h"
#include "core/jobs/JobSystem.h"
#include "core/models/AppState.h"
#include "core/models/DeletionImpact.h"
#include "debug/FileDebugger.h"
#include "ui/bootstrap/QmlContracts.h"
#include "ui/controllers/ActorController.h"
#include "ui/controllers/AnalysisController.h"
#include "ui/controllers/AnnualController.h"
#include "ui/controllers/ContractController.h"
#include "ui/controllers/ControllerContracts.h"
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
#include "ui/observability/Origins.h"
#include "ui/observability/Trace.h"
#include "ui/providers/DraftProofProvider.h"
#include "ui/state/StateFacade.h"
#include <QApplication>
#include <QQmlEngine>
#include <QQmlError>
#include <QUrl>

#include <cstdio>
#include <exception>
#include <memory>

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

core::controllers::exporting::ExportOptions::Format
toCoreExportFormat(ui::controllers::contracts::ExportFormat format) {
  switch (format) {
  case ui::controllers::contracts::ExportFormat::Csv:
    return core::controllers::exporting::ExportOptions::Format::Csv;
  case ui::controllers::contracts::ExportFormat::Xlsx:
    return core::controllers::exporting::ExportOptions::Format::Xlsx;
  default:
    ui::observability::reportFlow(
        core::errors::ErrorSeverity::Warning,
        core::errors::codes::UiFlowExportFallback,
        ui::observability::origins::app::kToCoreExportFormat,
        "Unknown export format, fallback to CSV",
        {{ui::observability::context::kFormat,
          std::to_string(static_cast<int>(format))}});
    return core::controllers::exporting::ExportOptions::Format::Csv;
  }
}

ui::exporting::ExportResult
executeExport(std::shared_ptr<const AppState> snapshot,
              const ui::exporting::ExportRequest &request) {
  core::controllers::exporting::ExportOptions options;
  options.outputPath = request.path.toStdString();
  options.includeFormulas = request.includeFormulas;
  options.locale = request.locale.toStdString();
  options.stateSnapshot = std::move(snapshot);
  options.requestedFormat = toCoreExportFormat(request.format);

  auto csv = std::make_shared<core::controllers::exporting::CsvController>();
  auto xlsx = std::make_shared<core::controllers::exporting::XlsxController>();
  core::controllers::exporting::ExportController exporter(xlsx, csv);
  exporter.exportData(options);

  return {options.status ==
              core::controllers::exporting::ExportOptions::Status::Ok,
          QString::fromStdString(options.errorCode),
          QString::fromStdString(options.message)};
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
  std::unique_ptr<AnalysisEngine> analysisEngine;
};

UiControllers setupUiControllers(
    QApplication &app, MainWindow &w, AppStateController &appStateCtrl,
    const std::shared_ptr<core::errors::IErrorReporter> &errorReporter) {
  UiControllers ui;

  ui.storage = new ui::StorageController(&appStateCtrl, &w);
  w.setQmlContextProperty(ui::qml::contracts::context::kStorageController,
                          ui.storage);

  const auto stateSnapshotProvider = [&appStateCtrl]() {
    return ui::exporting::createSnapshot(appStateCtrl.state());
  };

  ui.analysisEngine = std::make_unique<AnalysisEngine>();
  ui.annual = new ui::AnnualController(&appStateCtrl, &w);
  ui.actor = new ui::ActorController(&appStateCtrl, &w);
  ui.property = new ui::PropertyController(&appStateCtrl, &w);
  ui.contract = new ui::ContractController(&appStateCtrl, &w);
  ui.statement = new ui::StatementController(&appStateCtrl, &w);
  ui.transaction = new ui::TransactionController(&appStateCtrl, &w);
  ui.draft = new ui::DraftController(&appStateCtrl, &w);
  ui.analysisController = new ui::AnalysisController(
      &appStateCtrl, stateSnapshotProvider, ui.analysisEngine.get(), &w);
  w.setQmlContextProperty(ui::qml::contracts::context::kAnnualController,
                          ui.annual);
  w.setQmlContextProperty(ui::qml::contracts::context::kActorController,
                          ui.actor);
  w.setQmlContextProperty(ui::qml::contracts::context::kPropertyController,
                          ui.property);
  w.setQmlContextProperty(ui::qml::contracts::context::kContractController,
                          ui.contract);
  w.setQmlContextProperty(ui::qml::contracts::context::kStatementController,
                          ui.statement);
  w.setQmlContextProperty(ui::qml::contracts::context::kTransactionController,
                          ui.transaction);
  w.setQmlContextProperty(ui::qml::contracts::context::kDraftController,
                          ui.draft);
  w.setQmlContextProperty(ui::qml::contracts::context::kAnalysisController,
                          ui.analysisController);

  auto exportRunner =
      std::make_shared<ui::exporting::ExportRunner>(executeExport);
  ui.exportCtrl =
      new ui::ExportController(stateSnapshotProvider, exportRunner, &w);
  w.setQmlContextProperty(ui::qml::contracts::context::kExportController,
                          ui.exportCtrl);

  ui.language = new ui::LanguageController(&app, w.qmlEngine(), &w);
  w.setQmlContextProperty(ui::qml::contracts::context::kLanguageController,
                          ui.language);

  auto dbg = std::make_shared<FileDebugger>("", "import");
  auto popplerAdapter = createPopplerAdapter(dbg);
  auto opencvAdapter = createOpenCvAdapter(dbg);
  auto tesseractAdapter = createTesseractAdapter(dbg);

  auto poppler = api::poppler::createPopplerService(popplerAdapter);
  auto opencv = api::opencv::createOpenCvService(opencvAdapter);
  auto tesseract = api::tesseract::createTesseractService(tesseractAdapter);

  auto importSvc = createImportStatement(poppler, opencv, tesseract, dbg);
  auto stmtCtrl = std::make_shared<::StatementController>(importSvc);
  auto importCtrl = std::make_shared<::ImportController>(stmtCtrl);
  auto jobSystem = std::make_shared<core::jobs::JobSystem>(importCtrl);

  ui.import = new ui::ImportController(jobSystem, &w);
  ui.import->setErrorReporter(errorReporter);
  w.setQmlContextProperty(ui::qml::contracts::context::kImportController,
                          ui.import);

  w.addImageProvider(ui::qml::contracts::providers::kImportProof,
                     new ui::DraftProofProvider(ui.import));

  return ui;
}

void wireAppStateToSession(
    MainWindow &w, AppStateController &appStateCtrl,
    const std::shared_ptr<core::errors::IErrorReporter> &errorReporter) {
  if (w.dataSession()) {
    w.dataSession()->loadFromState(appStateCtrl.state());
  }

  appStateCtrl.setStateChangedCallback([&](const AppState &st) {
    if (w.dataSession()) {
      w.dataSession()->loadFromState(st);
    }
  });

  appStateCtrl.setDeletionImpactCallback([&](const DeletionImpact &impact) {
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
  QObject::connect(&w, &MainWindow::newFileRequested,
                   [&](const QString &path) { storage->newFile(path); });
  QObject::connect(&w, &MainWindow::openFileRequested,
                   [&](const QString &path) { storage->openFile(path); });
  QObject::connect(&w, &MainWindow::saveFileRequested,
                   [&]() { storage->saveFile(); });
  QObject::connect(&w, &MainWindow::saveFileAsRequested,
                   [&](const QString &path) { storage->saveFileAs(path); });
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
          event.code = core::errors::codes::QmlWarning;
          event.origin = ui::observability::origins::app::kQmlWarnings;
          event.message = warning.description().toStdString();
          event.context = {{ui::observability::context::kUrl,
                            warning.url().toString().toStdString()},
                           {ui::observability::context::kLine,
                            std::to_string(warning.line())},
                           {ui::observability::context::kColumn,
                            std::to_string(warning.column())}};
          errorReporter->report(event);
        }
      });
}

} // namespace

/**
 * @brief Initialize and run the QML-based UI.
 * @param app Reference to the already-created QApplication instance.
 * @param appStateCtrl Reference to the AppStateController that manages
 * application state.
 * @return Return value from `QApplication::exec()`.
 */
int startQmlApp(QApplication &app, AppStateController &appStateCtrl) {
  MainWindow w;

  auto errorReporter = core::errors::globalErrorReporter();
  if (!errorReporter) {
    errorReporter = std::make_shared<core::errors::DebuggerErrorReporter>(
        std::make_shared<FileDebugger>("", "errors"));
    core::errors::setGlobalErrorReporter(errorReporter);
  }
  appStateCtrl.setErrorReporter(errorReporter);

  const UiControllers ui =
      setupUiControllers(app, w, appStateCtrl, errorReporter);
  wireAppStateToSession(w, appStateCtrl, errorReporter);
  wireFileSignals(w, ui.storage);
  wireQmlWarnings(w, errorReporter);

  // Load QML after all context properties/providers are installed.
  w.loadQml();

  w.show();
  return app.exec();
}

#endif
