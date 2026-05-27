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
#include "core/ports/text-recognition/ITextRecognizer.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "debug/DebugDefaults.h"
#include "analysis-image-renderer/OpenCvAnalysisImageRendererAdapter.h"
#include "xlsx-writer/XlntTableWriterAdapter.h"
#include "ui/shell/AppContext.h"

using core::domain::DeletionImpact;
#include "archive/ZipArchiveAdapter.h"
#include "debug/ErrorReporter.h"
#include "debug/FileDebugger.h"
#include "ui/shell/QmlContracts.h"
#include "ui/platform/localization/LanguageService.h"
#include "ui/viewmodels/system/SettingsViewModel.h"
#include "ui/workflows/export/ExportRunner.h"
#include "ui/workflows/export/WorkspaceSnapshot.h"
#include "ui/shared/observability/ErrorCodes.h"
#include "ui/shared/observability/Origins.h"
#include "ui/shared/observability/Trace.h"
#include "ui/workflows/analysis/AnalysisWorkflow.h"
#include "ui/workflows/export/ExportWorkflow.h"
#include "ui/workflows/import/ImportWorkflow.h"
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
    txSnapshot.proofImageData = tx.proofImageData;
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
        "Unknown export format, defaulting to CSV",
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
  if (request.progressCallback) {
    exportRequest.progressCallback = [cb = request.progressCallback](double progress, const std::string &phase) {
      cb(progress, QString::fromStdString(phase));
    };
  }

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
      const QString objectType =
          item.value(QStringLiteral("objectType")).toString().trimmed().toLower();
      const QString objectId =
          item.value(QStringLiteral("objectId")).toString();
      if (objectId.isEmpty())
        continue;

      core::application::exporting::ExportObjectRequest objectRequest;
      objectRequest.objectId = objectId.toStdString();
      objectRequest.name = item.value(QStringLiteral("objectName")).toString().toStdString();
      objectRequest.annualId = item.value(QStringLiteral("annualId")).toString().toStdString();
      objectRequest.type = objectType == QStringLiteral("annual")
          ? core::application::exporting::ExportObjectType::Annual
          : core::application::exporting::ExportObjectType::Analysis;

      const QString normalized = item.value(QStringLiteral("exportType")).toString().trimmed().toLower();
      if (normalized == QStringLiteral("xlsx"))
        objectRequest.format = core::application::exporting::AnalysisExportFormat::Xlsx;
      else if (normalized == QStringLiteral("jpg") || normalized == QStringLiteral("jpeg"))
        objectRequest.format = core::application::exporting::AnalysisExportFormat::Jpg;
      else if (normalized == QStringLiteral("png"))
        objectRequest.format = core::application::exporting::AnalysisExportFormat::Png;
      else
        objectRequest.format = core::application::exporting::AnalysisExportFormat::Csv;

      exportRequest.objectRequests.push_back(std::move(objectRequest));
    }
  }

  core::application::exporting::ExportService exporter(
      std::move(archiveAdapter), std::move(xlsxWriter), std::move(imageRenderer));
  const auto result = exporter.exportData(exportRequest);
  return result;
}

struct UiServices {
  ui::AnalysisWorkflow *analysisWorkflow = nullptr;
  ui::ExportWorkflow *exportWorkflow = nullptr;
  ui::ImportWorkflow *importWorkflow = nullptr;
  ui::LanguageService *languageService = nullptr;
  std::shared_ptr<core::application::analysis::AnalysisService> analysisService;
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

  core::domain::catalog::WorkspaceCatalog mergeCatalogState(
      core::domain::catalog::WorkspaceCatalog primary,
      const core::domain::catalog::WorkspaceCatalog &secondary) const override {
    return core::application::importing::draft::mergeCatalogState(
        std::move(primary), secondary);
  }

  std::vector<std::string>
  referenceAliasesFromMetadata(const std::string &metadata) const override {
    return core::application::importing::draft::referenceAliasesFromMetadata(
        metadata);
  }
};

UiServices setupUiServices(
    QApplication &app, MainWindow &w,
    core::application::WorkspaceFacade &appStateFacade,
    const std::shared_ptr<core::errors::IErrorReporter> &errorReporter) {
  UiServices ui;

  if (w.workspace())
    w.workspace()->setCoreFacade(&appStateFacade);
  if (auto *appContext = w.appContext()) {
    appContext->setWorkspaceFacade(w.workspace());
  }
  w.setQmlContextProperty(ui::qml::contracts::context::kWorkspaceFacade,
                          w.workspace());

  const auto analysisSnapshotProvider = [&appStateFacade]() {
    return std::make_shared<const core::domain::catalog::WorkspaceCatalog>(appStateFacade.catalogState());
  };

  const auto exportSnapshotProvider = [&appStateFacade]() {
    return std::make_shared<const core::application::workspace::WorkspaceSessionState>(appStateFacade.state());
  };

  ui.analysisService =
      std::make_shared<core::application::analysis::AnalysisService>();
  auto analysisPresenter = std::make_shared<AnalysisPresenterAdapter>();
  auto analysisImageRenderer =
      std::make_shared<infra::analysis_image_renderer::OpenCvAnalysisImageRendererAdapter>();
  auto importMatcherService = std::make_shared<ImportMatcherServiceAdapter>();
  ui.analysisWorkflow =
      new ui::AnalysisWorkflow(&appStateFacade, analysisSnapshotProvider,
                               ui.analysisService, analysisPresenter, &w,
                               analysisImageRenderer);
  w.setQmlContextProperty(ui::qml::contracts::context::kAnalysisWorkflow,
                          ui.analysisWorkflow);
  if (auto *appContext = w.appContext())
    appContext->setAnalysisWorkflow(ui.analysisWorkflow);

  auto exportRunner =
      std::make_shared<ui::exporting::ExportRunner>(executeExport);
  auto exportPresenter = std::make_shared<ExportPresenterAdapter>();
  ui.exportWorkflow = new ui::ExportWorkflow(exportSnapshotProvider, exportRunner,
                                             exportPresenter, &w);
  ui.exportWorkflow->setExportLogsStore(
      [&appStateFacade](
          const std::vector<core::application::exporting::ExportLog> &logs) {
        appStateFacade.setExportLogs(toExportLogsCommand(logs));
      });
  w.setQmlContextProperty(ui::qml::contracts::context::kExportWorkflow,
                          ui.exportWorkflow);
  if (auto *appContext = w.appContext())
    appContext->setExportWorkflow(ui.exportWorkflow);

  ui.languageService = new ui::LanguageService(&app, w.qmlEngine(), &w);
  w.setQmlContextProperty(ui::qml::contracts::context::kLanguageService,
                          ui.languageService);
  if (auto *appContext = w.appContext())
    appContext->setLanguageService(ui.languageService);

  auto *settingsViewModel = new ui::SettingsViewModel(&w);
  w.setQmlContextProperty(ui::qml::contracts::context::kSettingsViewModel,
                          settingsViewModel);
  if (auto *appContext = w.appContext())
    appContext->setSettingsViewModel(settingsViewModel);

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

  const auto importSnapshotProvider = [&appStateFacade]() {
    return appStateFacade.state();
  };

  ui.importWorkflow =
      new ui::ImportWorkflow(importJobSystemFactory, errorReporter,
                             importPresenter, importMatcherService,
                             &appStateFacade, &w);
  ui.importWorkflow->setStateSnapshotProvider(importSnapshotProvider);
  ui.importWorkflow->setImportLogsStore(
      [&appStateFacade](
          const std::vector<core::application::importing::ImportLog> &logs) {
        appStateFacade.setImportLogs(toImportLogsCommand(logs));
      });
  ui.importWorkflow->setStatementDraftStore(
      [&appStateFacade](
          const core::application::importing::draft::StatementDraft &draft) {
        appStateFacade.saveStatementDraft({toStatementDraftSnapshot(draft)});
      });
  w.setQmlContextProperty(ui::qml::contracts::context::kImportWorkflow,
                          ui.importWorkflow);
  if (auto *appContext = w.appContext())
    appContext->setImportWorkflow(ui.importWorkflow);

  return ui;
}

void wireAppStateToSession(
    MainWindow &w, const UiServices &ui,
    core::application::WorkspaceFacade &appStateFacade,
    const std::shared_ptr<core::errors::IErrorReporter> &errorReporter) {
  if (w.workspace())
    w.workspace()->loadFromState(appStateFacade.state());

  appStateFacade.setStateChangedCallback(
      [&](const core::application::workspace::WorkspaceSessionState &document) {
        if (w.workspace())
          w.workspace()->loadFromState(document);
        if (ui.importWorkflow) {
          ui.importWorkflow->refreshFromStateSnapshot();
        }
        if (ui.exportWorkflow) {
          ui.exportWorkflow->refreshFromStateSnapshot();
        }
      });

  appStateFacade.setDeletionImpactCallback([&](const DeletionImpact &impact) {
    try {
      if (w.workspace())
        w.workspace()->applyDeletionImpact(impact);
    } catch (...) {
      if (errorReporter)
        errorReporter->reportException(
            core::errors::ErrorSeverity::Error,
            "app::wireAppStateToSession::applyDeletionImpact",
            std::current_exception());
    }
  });
}

void wireFileSignals(MainWindow &w, ui::WorkspaceFacade *workspace) {
  if (!workspace)
    return;

  QObject::connect(&w, &MainWindow::newFileRequested, workspace,
                   [workspace](const QString &path) { workspace->newFile(path); });
  QObject::connect(&w, &MainWindow::openFileRequested, workspace,
                   [workspace](const QString &path) { workspace->openFile(path); });
  QObject::connect(&w, &MainWindow::saveFileRequested, workspace,
                   [workspace]() { workspace->saveFile(); });
  QObject::connect(
      &w, &MainWindow::saveFileAsRequested, workspace,
      [workspace](const QString &path) { workspace->saveFileAs(path); });
  QObject::connect(workspace, &ui::WorkspaceFacade::operationSucceeded, &w,
                   &MainWindow::handleStorageOperationSucceeded);
  QObject::connect(workspace, &ui::WorkspaceFacade::operationFailed, &w,
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

  const UiServices ui =
      setupUiServices(app, w, appStateFacade, errorReporter);

  wireAppStateToSession(w, ui, appStateFacade, errorReporter);

  if (ui.importWorkflow) {
    ui.importWorkflow->refreshFromStateSnapshot();
  }
  if (ui.exportWorkflow) {
    ui.exportWorkflow->refreshFromStateSnapshot();
  }

  wireFileSignals(w, w.workspace());

  wireQmlWarnings(w, errorReporter);

  // Load QML after all context properties/providers are installed.
  w.loadQml();

  if (w.workspace()) {
    w.workspace()->loadFromState(appStateFacade.state());
  }
  if (ui.importWorkflow) {
    ui.importWorkflow->refreshFromStateSnapshot();
  }
  if (ui.exportWorkflow) {
    ui.exportWorkflow->refreshFromStateSnapshot();
  }

  w.show();
  const int exitCode = app.exec();

  appStateFacade.setStateChangedCallback({});
  appStateFacade.setDeletionImpactCallback({});
  appStateFacade.setErrorReporter({});

  return exitCode;
}

#endif
