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

#ifdef USE_QML
#include "MainWindow.h"
#include "ui/controllers/UiStatementController.h"
#include "ui/controllers/UiFileController.h"
#include "debug/FileDebugger.h"
#include "api/poppler/IPopplerService.h"
#include "api/opencv/IOpenCvService.h"
#include "api/tesseract/ITesseractService.h"
#include "core/import/IImportStatement.h"
#include <QList>
#include <QVariant>

std::shared_ptr<api::poppler::IPopplerAdapter> createPopplerAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<api::opencv::IOpenCvAdapter> createOpenCvAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<api::tesseract::ITesseractAdapter> createTesseractAdapter(std::shared_ptr<IDebugger> dbg);
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

#ifdef USE_QML
    MainWindow w;

    auto uiFileCtrl = new UiFileController(&fileCtrl, &w);
    w.setQmlContextProperty("uiFileController", uiFileCtrl);

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

    auto fileDbg = std::make_shared<FileDebugger>(std::string("debug_output"));

    auto popplerAdapter = createPopplerAdapter(fileDbg);
    auto openCvAdapter = createOpenCvAdapter(fileDbg);
    auto tesseractAdapter = createTesseractAdapter(fileDbg);

    auto popplerSvc = api::poppler::createPopplerService(popplerAdapter);
    auto openCvSvc = api::opencv::createOpenCvService(openCvAdapter);
    auto tesseractSvc = api::tesseract::createTesseractService(tesseractAdapter);

    auto importer = createImportStatement(popplerSvc, openCvSvc, tesseractSvc);
    auto coreCtrl = std::make_shared<StatementController>(importer);

    auto uiCtrl = new UiStatementController(coreCtrl, &w);
    QObject::connect(&w, &MainWindow::importRequested, uiCtrl, &UiStatementController::importStatement);
    QObject::connect(uiCtrl, &UiStatementController::transactionsExtracted, [](const QList<QVariant>& tx){
        qDebug() << "transactionsExtracted count:" << tx.size();
    });

    w.show();
    int result = app.exec();
    return result;
#else
    return -1;
#endif
}