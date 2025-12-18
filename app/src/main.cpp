#include "app/pch.h"
#include "core/utils/Environment.h"
#include <QApplication>

#include "core/repositories/IConfigRepository.h"
#include "persistence/Factory.h"
#include "core/models/Config.h"
#include "core/managers/ConfigManager.h"
#include "core/controllers/StatementController.h"

#include <QDir>
#include <QDebug>

#ifdef USE_QML
#include "MainWindow.h"
#include "ui/controllers/QTStatementController.h"
#include "debug/FileDebugger.h"
#include "api/poppler/IPopplerService.h"
#include "api/opencv/IOpenCvService.h"
#include "api/tesseract/ITesseractService.h"
#include "core/import/IImportStatement.h"
#include <QList>
#include <QVariant>

// Adapter factory functions are implemented in the services; declare them here
std::shared_ptr<api::poppler::IPopplerAdapter> createPopplerAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<api::opencv::IOpenCvAdapter> createOpenCvAdapter(std::shared_ptr<IDebugger> dbg);
std::shared_ptr<api::tesseract::ITesseractAdapter> createTesseractAdapter(std::shared_ptr<IDebugger> dbg);
#endif

// Simple message handler to ensure Qt messages (incl. QML warnings) are visible in the console
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

    const std::string dbPath = "fossredder.db";
    auto actorRepo = createSqliteActorRepository(dbPath);
    auto propRepo = createSqlitePropertyRepository(dbPath);
    auto stmtRepo = createSqliteStatementRepository(dbPath);
    auto cfgRepo = createSqliteConfigRepository(dbPath);
    auto txRepo = createSqliteTransactionRepository(dbPath);
    auto bgRepo = createSqliteBookingGroupRepository(dbPath);

    ConfigManager cfgMgr;
    if (auto def = cfgRepo->getDefaultConfig())
        cfgMgr.setConfig(*def);

    QApplication app(argc, argv);
    app.setStyle("Fusion");

#ifdef USE_QML
    // Setup core services and controllers and wire to UI
    MainWindow w;

    // Create debugger backend for services (writes debug artifacts to disk)
    auto fileDbg = std::make_shared<FileDebugger>(std::string("debug_output"));

    // Create adapters (implementations exist in services/*/Adapter.cpp)
    auto popplerAdapter = createPopplerAdapter(fileDbg);
    auto openCvAdapter = createOpenCvAdapter(fileDbg);
    auto tesseractAdapter = createTesseractAdapter(fileDbg);

    // Create service wrappers
    auto popplerSvc = api::poppler::createPopplerService(popplerAdapter);
    auto openCvSvc = api::opencv::createOpenCvService(openCvAdapter);
    auto tesseractSvc = api::tesseract::createTesseractService(tesseractAdapter);

    // Create extraction service and core controller
    auto importer = createImportStatement(popplerSvc, openCvSvc, tesseractSvc);
    auto coreCtrl = std::make_shared<StatementController>(importer);

    // Create QT wrapper controller and connect UI
    auto qtCtrl = new QTStatementController(coreCtrl, &w);
    QObject::connect(&w, &MainWindow::importRequested, qtCtrl, &QTStatementController::importStatement);
    QObject::connect(qtCtrl, &QTStatementController::transactionsExtracted, [](const QList<QVariant>& tx){
        qDebug() << "transactionsExtracted count:" << tx.size();
    });

    w.show();
    int result = app.exec();
    return result;
#else
    return -1;
#endif
}