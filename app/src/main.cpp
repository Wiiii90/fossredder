/**
 * @file main.cpp
 * @brief Application bootstrap: environment, repositories and QApplication initialization.
 */

#include "core/utils/Environment.h"
#include <QApplication>

#include "persistence/Factory.h"
#include "persistence/AppStateStore.h"
#include "core/repositories/IConfigRepository.h"
#include "core/managers/ConfigManager.h"
#include "core/managers/StorageManager.h"
#include "core/controllers/AppStateController.h"

#include <QDir>

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
extern int startQmlApp(QApplication& app, AppStateController& appStateCtrl);
#endif

int main(int argc, char* argv[]) {
    // Install global Qt message handler early so startup logs are captured
    qInstallMessageHandler(qtMessageHandler);

    // Load runtime environment from .env if present
    env::load_dotenv(".env", false);

    // Setup storage manager and controller (manages application state files)
    const std::string appDataRoot = QDir::homePath().toStdString() + std::string("/.fossredder");
    StorageManager sm(appDataRoot);
    auto smPtr = std::make_unique<StorageManager>(std::move(sm));
    AppStateController appStateCtrl(std::move(smPtr));

    // Initialize configuration repository (uses persistence factory)
    // Use a per-user path so the installed app does not attempt to write into Program Files.
    auto cfgRepo = createSqliteConfigRepository((std::filesystem::path(appDataRoot) / "fossredder.db").string());

    appStateCtrl.setRepoFactory([](const std::string& dbPath) {
        auto db = createSqliteDb(dbPath);
        IStorageManager::Repositories r;
        r.actors = createSqliteActorRepository(db);
        r.properties = createSqlitePropertyRepository(db);
        r.contracts = createSqliteContractRepository(db);
        r.statements = createSqliteStatementRepository(db);
        r.transactions = createSqliteTransactionRepository(db);
        return r;
    });
    appStateCtrl.setAtomicStoreLoad([](const std::string& dbPath) {
        auto db = createSqliteDb(dbPath);
        AppStateStore store(db);
        return store.load();
    });
    appStateCtrl.setAtomicStoreSave([](const std::string& dbPath, const AppState& state) {
        auto db = createSqliteDb(dbPath);
        AppStateStore store(db);
        auto res = store.save(state);
        return res.impact;
    });
    appStateCtrl.openLatest();
    if (appStateCtrl.currentPath().empty()) {
        appStateCtrl.newFile((std::filesystem::path(appDataRoot) / "fossredder.db").string());
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
    return startQmlApp(app, appStateCtrl);
#else
    // No UI available in this build configuration
    return 0;
#endif
}