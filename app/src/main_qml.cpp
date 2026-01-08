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
#include "ui/controllers/UiImportController.h"
#include "ui/controllers/UiStorageController.h"
#include "ui/controllers/UiDomainController.h"
#include "ui/state/UiDataSession.h"
#include "debug/FileDebugger.h"
#include "core/controllers/ImportController.h"
#include "core/controllers/StatementController.h"
#include "core/import/IImportStatement.h"
#include "api/poppler/IPopplerAdapter.h"
#include "api/opencv/IOpenCvAdapter.h"
#include "api/tesseract/ITesseractAdapter.h"
#include "api/poppler/IPopplerService.h"
#include "api/opencv/IOpenCvService.h"
#include "api/tesseract/ITesseractService.h"

#include <memory>

std::shared_ptr<api::poppler::IPopplerAdapter> createPopplerAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<api::opencv::IOpenCvAdapter> createOpenCvAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<api::tesseract::ITesseractAdapter> createTesseractAdapter(std::shared_ptr<IDebugger> dbg);

namespace api { namespace poppler { std::shared_ptr<IPopplerService> createPopplerService(std::shared_ptr<IPopplerAdapter> adapter); } }
namespace api { namespace opencv { std::shared_ptr<IOpenCvService> createOpenCvService(std::shared_ptr<IOpenCvAdapter> adapter); } }
namespace api { namespace tesseract { std::shared_ptr<ITesseractService> createTesseractService(std::shared_ptr<ITesseractAdapter> adapter); } }

/**
 * @brief Initialize and run the QML-based UI.
 * @param app Reference to the already-created QApplication instance.
 * @param appStateCtrl Reference to the AppStateController that manages application state.
 * @return Return value from `QApplication::exec()`.
 */
int startQmlApp(QApplication& app, AppStateController& appStateCtrl) {
    // Register Theme singleton so QML can import as `import FossRedder 1.0;` then use `Theme`
    qmlRegisterSingletonType(QUrl("qrc:/qml/theme/Theme.qml"), "FossRedder", 1, 0, "Theme");

    MainWindow w;

    auto uiFileCtrl = new UiStorageController(&appStateCtrl, &w);
    w.setQmlContextProperty("uiFileController", uiFileCtrl);

    auto uiDomain = new UiDomainController(&appStateCtrl, &w);
    w.setQmlContextProperty("uiDomain", uiDomain);

    {
        auto dbg = std::make_shared<FileDebugger>("", "import");
        auto popplerAdapter = createPopplerAdapter(dbg);
        auto opencvAdapter = createOpenCvAdapter(dbg);
        auto tesseractAdapter = createTesseractAdapter(dbg);

        auto poppler = api::poppler::createPopplerService(popplerAdapter);
        auto opencv = api::opencv::createOpenCvService(opencvAdapter);
        auto tesseract = api::tesseract::createTesseractService(tesseractAdapter);

        auto importSvc = createImportStatement(poppler, opencv, tesseract, dbg);
        auto stmtCtrl = std::make_shared<StatementController>(importSvc);
        auto importCtrl = std::make_shared<ImportController>(stmtCtrl);

        auto uiImport = new UiImportController(importCtrl, &w);
        uiImport->setDomainController(uiDomain);
        w.setQmlContextProperty("uiImport", uiImport);
    }

    if (w.dataSession()) {
        w.dataSession()->loadFromState(appStateCtrl.state());
    }

    appStateCtrl.setStateChangedCallback([&](const AppState& st) {
        if (w.dataSession()) {
            w.dataSession()->loadFromState(st);
        }
    });

    QObject::connect(&w, &MainWindow::newFileRequested, [&](const QString& path){
        uiFileCtrl->newFile(path);
    });
    QObject::connect(&w, &MainWindow::openFileRequested, [&](const QString& path){
        uiFileCtrl->openFile(path);
    });
    QObject::connect(&w, &MainWindow::saveFileRequested, [&](){
        uiFileCtrl->saveFile();
    });
    QObject::connect(&w, &MainWindow::saveFileAsRequested, [&](const QString& path){
        uiFileCtrl->saveFileAs(path);
    });

    w.show();
    return app.exec();
}

#endif
