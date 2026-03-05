/**
 * @file main_qml.cpp
 * @brief QML startup implementation.
 *
 * This translation unit implements `startQmlApp` which initializes the main window,
 * registers UI controllers and services, connects signals/slots and starts the Qt event loop.
 */

#ifdef USE_QML
#include <QApplication>
#include <QQmlEngine>
#include <QQmlError>
#include <QUrl>
#include "MainWindow.h"
#include "ui/controllers/AnalysisController.h"
#include "ui/controllers/ActorController.h"
#include "ui/controllers/AnnualController.h"
#include "ui/controllers/ContractController.h"
#include "ui/controllers/DraftController.h"
#include "ui/controllers/ExportController.h"
#include "ui/controllers/ImportController.h"
#include "ui/controllers/PropertyController.h"
#include "ui/controllers/StatementController.h"
#include "ui/controllers/StorageController.h"
#include "ui/controllers/TransactionController.h"
#include "ui/state/StateFacade.h"
#include "core/errors/DebuggerErrorReporter.h"
#include "core/errors/ErrorCodes.h"
#include "debug/FileDebugger.h"
#include "core/controllers/ImportController.h"
#include "core/controllers/StatementController.h"
#include "core/import/IImportStatement.h"
#include "core/jobs/JobSystem.h"
#include "core/analysis/AnalysisController.h"
#include "api/poppler/IPopplerAdapter.h"
#include "api/opencv/IOpenCvAdapter.h"
#include "api/tesseract/ITesseractAdapter.h"
#include "api/poppler/IPopplerService.h"
#include "api/opencv/IOpenCvService.h"
#include "api/tesseract/ITesseractService.h"
#include "core/models/DeletionImpact.h"
#include "ui/providers/DraftProofProvider.h"

#include <memory>
#include <cstdio>
#include <exception>

std::shared_ptr<api::poppler::IPopplerAdapter> createPopplerAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<api::opencv::IOpenCvAdapter> createOpenCvAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<api::tesseract::ITesseractAdapter> createTesseractAdapter(std::shared_ptr<IDebugger> dbg);

namespace api { namespace poppler { std::shared_ptr<IPopplerService> createPopplerService(std::shared_ptr<IPopplerAdapter> adapter); } }
namespace api { namespace opencv { std::shared_ptr<IOpenCvService> createOpenCvService(std::shared_ptr<IOpenCvAdapter> adapter); } }
namespace api { namespace tesseract { std::shared_ptr<ITesseractService> createTesseractService(std::shared_ptr<ITesseractAdapter> adapter); } }

namespace {

struct UiControllers {
    ui::StorageController* storage = nullptr;
    ui::AnnualController* annual = nullptr;
    ui::ActorController* actor = nullptr;
    ui::PropertyController* property = nullptr;
    ui::ContractController* contract = nullptr;
    ui::StatementController* statement = nullptr;
    ui::TransactionController* transaction = nullptr;
    ui::DraftController* draft = nullptr;
    ui::AnalysisController* analysisUi = nullptr;
    ui::ExportController* exportCtrl = nullptr;
    ui::ImportController* import = nullptr;
    std::unique_ptr<AnalysisController> analysis;
};

UiControllers setupUiControllers(MainWindow& w, AppStateController& appStateCtrl, const std::shared_ptr<core::errors::IErrorReporter>& errorReporter)
{
    UiControllers ui;

    ui.storage = new ui::StorageController(&appStateCtrl, &w);
    w.setQmlContextProperty("storageController", ui.storage);

    ui.analysis = std::make_unique<AnalysisController>();
    ui.annual = new ui::AnnualController(&appStateCtrl, &w);
    ui.actor = new ui::ActorController(&appStateCtrl, &w);
    ui.property = new ui::PropertyController(&appStateCtrl, &w);
    ui.contract = new ui::ContractController(&appStateCtrl, &w);
    ui.statement = new ui::StatementController(&appStateCtrl, &w);
    ui.transaction = new ui::TransactionController(&appStateCtrl, &w);
    ui.draft = new ui::DraftController(&appStateCtrl, &w);
    ui.analysisUi = new ui::AnalysisController(&appStateCtrl, ui.analysis.get(), &w);
    w.setQmlContextProperty("annualController", ui.annual);
    w.setQmlContextProperty("actorController", ui.actor);
    w.setQmlContextProperty("propertyController", ui.property);
    w.setQmlContextProperty("contractController", ui.contract);
    w.setQmlContextProperty("statementController", ui.statement);
    w.setQmlContextProperty("transactionController", ui.transaction);
    w.setQmlContextProperty("draftController", ui.draft);
    w.setQmlContextProperty("analysisController", ui.analysisUi);

    ui.exportCtrl = new ui::ExportController(&appStateCtrl, &w);
    w.setQmlContextProperty("exportController", ui.exportCtrl);

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
    w.setQmlContextProperty("importController", ui.import);

    w.addImageProvider(QStringLiteral("importProof"), new ui::DraftProofProvider(ui.import));

    return ui;
}

void wireAppStateToSession(MainWindow& w, AppStateController& appStateCtrl, const std::shared_ptr<core::errors::IErrorReporter>& errorReporter)
{
    if (w.dataSession()) {
        w.dataSession()->loadFromState(appStateCtrl.state());
    }

    appStateCtrl.setStateChangedCallback([&](const AppState& st) {
        if (w.dataSession()) {
            w.dataSession()->loadFromState(st);
        }
    });

    appStateCtrl.setDeletionImpactCallback([&](const DeletionImpact& impact){
        try {
            if (w.dataSession()) w.dataSession()->applyDeletionImpact(impact);
        } catch (...) {
            if (errorReporter) errorReporter->reportException(core::errors::ErrorSeverity::Error, "app::wireAppStateToSession::applyDeletionImpact", std::current_exception());
        }
    });
}

void wireFileSignals(MainWindow& w, ui::StorageController* storage)
{
    QObject::connect(&w, &MainWindow::newFileRequested, [&](const QString& path){
        storage->newFile(path);
    });
    QObject::connect(&w, &MainWindow::openFileRequested, [&](const QString& path){
        storage->openFile(path);
    });
    QObject::connect(&w, &MainWindow::saveFileRequested, [&](){
        storage->saveFile();
    });
    QObject::connect(&w, &MainWindow::saveFileAsRequested, [&](const QString& path){
        storage->saveFileAs(path);
    });
}

void wireQmlWarnings(MainWindow& w, const std::shared_ptr<core::errors::IErrorReporter>& errorReporter)
{
    auto* engine = w.qmlEngine();
    if (!engine || !errorReporter) return;

    QObject::connect(engine, &QQmlEngine::warnings, &w, [errorReporter](const QList<QQmlError>& warnings) {
        for (const auto& warning : warnings) {
            core::errors::ErrorEvent event;
            event.severity = core::errors::ErrorSeverity::Warning;
            event.code = core::errors::codes::QmlWarning;
            event.origin = "app::qml::warnings";
            event.message = warning.description().toStdString();
            event.context = {
                {"url", warning.url().toString().toStdString()},
                {"line", std::to_string(warning.line())},
                {"column", std::to_string(warning.column())}
            };
            errorReporter->report(event);
        }
    });
}

}

/**
 * @brief Initialize and run the QML-based UI.
 * @param app Reference to the already-created QApplication instance.
 * @param appStateCtrl Reference to the AppStateController that manages application state.
 * @return Return value from `QApplication::exec()`.
 */
int startQmlApp(QApplication& app, AppStateController& appStateCtrl) {
    MainWindow w;

    auto errorReporter = std::make_shared<core::errors::DebuggerErrorReporter>(
        std::make_shared<FileDebugger>("", "errors"));
    appStateCtrl.setErrorReporter(errorReporter);

    const UiControllers ui = setupUiControllers(w, appStateCtrl, errorReporter);
    wireAppStateToSession(w, appStateCtrl, errorReporter);
    wireFileSignals(w, ui.storage);
    wireQmlWarnings(w, errorReporter);

    // Load QML after all context properties/providers are installed.
    w.loadQml();

    w.show();
    return app.exec();
}

#endif
