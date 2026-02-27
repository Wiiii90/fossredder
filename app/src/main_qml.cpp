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
#include <QUrl>
#include "MainWindow.h"
#include "ui/controllers/AnnualController.h"
#include "ui/controllers/ActorController.h"
#include "ui/controllers/AnalysisController.h"
#include "ui/controllers/ContractController.h"
#include "ui/controllers/DraftController.h"
#include "ui/controllers/ExportController.h"
#include "ui/controllers/ImportController.h"
#include "ui/controllers/PropertyController.h"
#include "ui/controllers/StatementController.h"
#include "ui/controllers/StorageController.h"
#include "ui/controllers/TransactionController.h"
#include "ui/state/UiDataSession.h"
#include "debug/FileDebugger.h"
#include "core/controllers/ImportController.h"
#include "core/controllers/StatementController.h"
#include "core/import/IImportStatement.h"
#include "core/jobs/JobSystem.h"
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
    ui::AnalysisController* analysis = nullptr;
    ui::ExportController* exportCtrl = nullptr;
    ui::ImportController* import = nullptr;
};

UiControllers setupUiControllers(MainWindow& w, AppStateController& appStateCtrl)
{
    UiControllers ui;

    ui.storage = new ui::StorageController(&appStateCtrl, &w);
    w.setQmlContextProperty("storageController", ui.storage);

    ui.annual = new ui::AnnualController(&appStateCtrl, &w);
    w.setQmlContextProperty("annualController", ui.annual);

    ui.actor = new ui::ActorController(&appStateCtrl, &w);
    w.setQmlContextProperty("actorController", ui.actor);

    ui.property = new ui::PropertyController(&appStateCtrl, &w);
    w.setQmlContextProperty("propertyController", ui.property);

    ui.contract = new ui::ContractController(&appStateCtrl, &w);
    w.setQmlContextProperty("contractController", ui.contract);

    ui.statement = new ui::StatementController(&appStateCtrl, &w);
    w.setQmlContextProperty("statementController", ui.statement);

    ui.transaction = new ui::TransactionController(&appStateCtrl, &w);
    w.setQmlContextProperty("transactionController", ui.transaction);

    ui.draft = new ui::DraftController(&appStateCtrl, &w);
    w.setQmlContextProperty("draftController", ui.draft);

    ui.analysis = new ui::AnalysisController(&appStateCtrl, &w);
    w.setQmlContextProperty("analysisController", ui.analysis);

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
    w.setQmlContextProperty("importController", ui.import);

    w.addImageProvider(QStringLiteral("importProof"), new DraftProofProvider(ui.import));

    return ui;
}

void wireAppStateToSession(MainWindow& w, AppStateController& appStateCtrl)
{
    if (w.dataSession()) {
        w.dataSession()->loadFromState(appStateCtrl.state());
    }

    appStateCtrl.setStateChangedCallback([&](const AppState& st) {
        if (w.dataSession()) {
            w.dataSession()->loadFromState(st);
        }
    });

    appStateCtrl.setTransactionsChangedCallback([&](const std::vector<std::string>& ids){
        try {
            if (w.dataSession()) w.dataSession()->applyTransactionUpdates(ids, appStateCtrl.state());
        } catch (...) {}
    });

    appStateCtrl.setDeletionImpactCallback([&](const DeletionImpact& impact){
        try {
            if (w.dataSession()) w.dataSession()->applyDeletionImpact(impact);
        } catch (...) {}
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

}

/**
 * @brief Initialize and run the QML-based UI.
 * @param app Reference to the already-created QApplication instance.
 * @param appStateCtrl Reference to the AppStateController that manages application state.
 * @return Return value from `QApplication::exec()`.
 */
int startQmlApp(QApplication& app, AppStateController& appStateCtrl) {
    MainWindow w;

    const UiControllers ui = setupUiControllers(w, appStateCtrl);
    wireAppStateToSession(w, appStateCtrl);
    wireFileSignals(w, ui.storage);

    // Load QML after all context properties/providers are installed.
    w.loadQml();

    w.show();
    return app.exec();
}

#endif
