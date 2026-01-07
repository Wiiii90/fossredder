#include "app/pch.h"
#include "core/utils/Environment.h"
#include <QApplication>

#include "persistence/Factory.h"
#include "persistence/AppStateStore.h"
#include "core/repositories/IConfigRepository.h"
#include "core/managers/ConfigManager.h"
#include "core/managers/FileManager.h"
#include "core/controllers/FileController.h"

#include <QDir>

/**
 * @file main.cpp
 * @brief Application bootstrap: environment, repositories and QApplication initialization.
 *
 * This translation unit performs global startup tasks (load .env, initialize repositories
 * and file manager) and then delegates to the QML UI startup when compiled with USE_QML.
 */

/**
 * @brief Global Qt message handler that redirects Qt logging to stderr with context.
 */
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

#ifdef USE_QML
/**
 * @brief Start the QML application UI.
 *
 * Implemented in `main_qml.cpp`. Only available when built with USE_QML.
 */
extern int startQmlApp(QApplication& app, FileController& fileCtrl);
#endif

int main(int argc, char* argv[]) {
    // Install global Qt message handler early so startup logs are captured
    qInstallMessageHandler(qtMessageHandler);

    // Load runtime environment from .env if present
    env::load_dotenv(".env", false);

    // Initialize configuration repository (uses persistence factory)
    auto cfgRepo = createSqliteConfigRepository("fossredder.db");

    // Setup file manager and controller (manages application state files)
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

    // Apply default configuration if available
    ConfigManager cfgMgr;
    if (auto def = cfgRepo->getDefaultConfig())
        cfgMgr.setConfig(*def);

    // Create the Qt application (manages event loop and GUI resources)
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    // Ensure Qt finds deployed plugins and QML modules next to the executable
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());

#ifdef USE_QML
    // Delegate to QML-specific startup
    return startQmlApp(app, fileCtrl);
#else
    // No UI available in this build configuration
    return 0;
#endif
}