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
#include "ui/controllers/DomainController.h"
#include "ui/controllers/ExportController.h"
#include "ui/controllers/ImportController.h"
#include "ui/controllers/StorageController.h"
#include "ui/state/StateFacade.h"
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
    ui::DomainController* domain = nullptr;
    ui::ExportController* exportCtrl = nullptr;
    ui::ImportController* import = nullptr;
};

UiControllers setupUiControllers(MainWindow& w, AppStateController& appStateCtrl)
{
    UiControllers ui;

    ui.storage = new ui::StorageController(&appStateCtrl, &w);
    w.setQmlContextProperty("storageController", ui.storage);

    ui.domain = new ui::DomainController(&appStateCtrl, &w);
    w.setQmlContextProperty("domainController", ui.domain);
    w.setQmlContextProperty("annualController", ui.domain);
    w.setQmlContextProperty("actorController", ui.domain);
    w.setQmlContextProperty("propertyController", ui.domain);
    w.setQmlContextProperty("contractController", ui.domain);
    w.setQmlContextProperty("statementController", ui.domain);
    w.setQmlContextProperty("transactionController", ui.domain);
    w.setQmlContextProperty("draftController", ui.domain);
    w.setQmlContextProperty("analysisController", ui.domain);

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

    w.addImageProvider(QStringLiteral("importProof"), new ui::DraftProofProvider(ui.import));

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
