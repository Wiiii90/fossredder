#include "app/pch.h"
#include "core/utils/Environment.h"
#include <QApplication>

#include "core/repositories/IConfigRepository.h"
#include "persistence/Factory.h"
#include "core/models/Config.h"
#include "core/managers/ConfigManager.h"
#include "core/controllers/StatementController.h"
#include "core/managers/FileManager.h"
#include "core/controllers/FileController.h"
#include "persistence/AppStateStore.h"

#include <QDir>
#include <QDebug>
#include <QCoreApplication>

#ifdef USE_QML
#include "MainWindow.h"
#include "ui/controllers/UiImportController.h"
#include "ui/controllers/UiFileController.h"
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
#include <QList>
#include <QVariant>

std::shared_ptr<api::poppler::IPopplerAdapter> createPopplerAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<api::opencv::IOpenCvAdapter> createOpenCvAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<api::tesseract::ITesseractAdapter> createTesseractAdapter(std::shared_ptr<IDebugger> dbg);

namespace api { namespace poppler { std::shared_ptr<IPopplerService> createPopplerService(std::shared_ptr<IPopplerAdapter> adapter); } }
namespace api { namespace opencv { std::shared_ptr<IOpenCvService> createOpenCvService(std::shared_ptr<IOpenCvAdapter> adapter); } }
namespace api { namespace tesseract { std::shared_ptr<ITesseractService> createTesseractService(std::shared_ptr<ITesseractAdapter> adapter); } }
#endif

static void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        abort();
    }
}

int main(int argc, char* argv[]) {
    qInstallMessageHandler(qtMessageHandler);

    env::load_dotenv(".env", false);

    auto cfgRepo = createSqliteConfigRepository("fossredder.db");

    FileManager fm(QDir::homePath().toStdString() + std::string("/.fossredder"));
    FileController fileCtrl(std::move(fm));
    fileCtrl.setRepoFactory([](const std::string& dbPath) {
        auto db = createSqliteDb(dbPath);
        FileManager::Repositories r;
        r.actors = createSqliteActorRepository(db);
        r.properties = createSqlitePropertyRepository(db);
        r.contracts = createSqliteContractRepository(db);
        r.statements = createSqliteStatementRepository(db);
        r.transactions = createSqliteTransactionRepository(db);
        return r;
    });
    fileCtrl.setAtomicStoreLoad([](const std::string& dbPath) {
        auto db = createSqliteDb(dbPath);
        AppStateStore store(db);
        return store.load();
    });
    fileCtrl.setAtomicStoreSave([](const std::string& dbPath, const AppState& state) {
        auto db = createSqliteDb(dbPath);
        AppStateStore store(db);
        auto res = store.save(state);
        return res.impact;
    });
    fileCtrl.openLatest();
    if (fileCtrl.currentPath().empty()) {
        fileCtrl.newFile("fossredder.db");
    }

    ConfigManager cfgMgr;
    if (auto def = cfgRepo->getDefaultConfig())
        cfgMgr.setConfig(*def);

    QApplication app(argc, argv);
    app.setStyle("Fusion");

    // Ensure Qt finds deployed plugins next to the executable (qt.conf sets Plugins=./).
    // Some run configurations still miss the runtime library path without this.
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());

#ifdef USE_QML
    MainWindow w;

    auto uiFileCtrl = new UiFileController(&fileCtrl, &w);
    w.setQmlContextProperty("uiFileController", uiFileCtrl);

    auto uiDomain = new UiDomainController(&fileCtrl, &w);
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
        w.dataSession()->loadFromState(fileCtrl.state());
    }

    fileCtrl.setStateChangedCallback([&](const AppState& st) {
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
#else
    return -1;
#endif
}