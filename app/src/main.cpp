#include "core/utils/Environment.h"
#include <QApplication>
#include <QMessageBox>
#include <QObject>
#include <QQuickStyle>

#include "persistence/Factory.h"
#include "persistence/AppStateStore.h"
#include "core/repositories/IConfigRepository.h"
#include "core/managers/ConfigManager.h"
#include "core/managers/StorageManager.h"
#include "core/controllers/AppStateController.h"

#include <QDir>
#include <filesystem>
#include <cstdio>

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

    // Ensure Qt Quick Controls uses a non-native style that supports customization
    // Call before creating the QApplication/QGuiApplication
    QQuickStyle::setStyle("Fusion");

    // Create the Qt application (manages event loop and GUI resources)
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    // Setup storage manager and controller (manages application state files)
    const std::string appDataRoot = QDir::homePath().toStdString() + std::string("/.fossredder");
    StorageManager sm(appDataRoot);
    auto smPtr = std::make_unique<StorageManager>(std::move(sm));
    AppStateController appStateCtrl(std::move(smPtr));

    // Initialize configuration repository (uses persistence factory)
    // Use a per-user path so the installed app does not attempt to write into Program Files.
    std::string cfgDbPath = (std::filesystem::path(appDataRoot) / "fossredder.db").string();

    // ensure parent directory exists
    try {
        std::filesystem::path p(cfgDbPath);
        if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());
    } catch (const std::exception& ex) {
        fprintf(stderr, "Warning: could not create db parent directories: %s\n", ex.what());
    }

    std::shared_ptr<IConfigRepository> cfgRepo;
    try {
        auto cfgDb = createSqliteDb(cfgDbPath);
        cfgRepo = createSqliteConfigRepository(cfgDb);
    } catch (const std::exception& ex) {
        fprintf(stderr, "Warning: failed to open config DB '%s': %s\n", cfgDbPath.c_str(), ex.what());
        cfgRepo = nullptr;
    }

    appStateCtrl.setRepoFactory([&](const std::string& dbPath) {
        // ensure directory exists for requested dbPath
        try {
            std::filesystem::path p(dbPath);
            if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());
        } catch (...) {}
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

    try {
        appStateCtrl.openLatest();
    } catch (const std::exception& ex) {
        fprintf(stderr, "Warning: failed to open latest app state: %s\n", ex.what());
        // continue with empty state
    }

    // Only create a new file if no path was found AND the loaded state is empty.
    // This avoids accidentally overwriting a valid loaded state due to
    // registry or ordering issues at startup.
    if (appStateCtrl.currentPath().empty()
        && appStateCtrl.state().actors.empty()
        && appStateCtrl.state().properties.empty()
        && appStateCtrl.state().contracts.empty()
        && appStateCtrl.state().statements.empty()
        && appStateCtrl.state().transactions.empty()) {
        appStateCtrl.newFile((std::filesystem::path(appDataRoot) / "fossredder.db").string());
    }

    // Apply default configuration if available
    ConfigManager cfgMgr;
    if (cfgRepo) {
        try {
            if (auto def = cfgRepo->getDefaultConfig())
                cfgMgr.setConfig(*def);
        } catch (const std::exception& ex) {
            fprintf(stderr, "Warning: failed to read default config: %s\n", ex.what());
        }
    }

    // Ensure Qt finds deployed plugins and QML modules next to the executable
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());

#ifdef USE_QML
    // Delegate to QML-specific startup
    try {
        return startQmlApp(app, appStateCtrl);
    } catch (const std::exception& ex) {
        fprintf(stderr, "Fatal error during QML startup: %s\n", ex.what());
        QMessageBox::critical(nullptr, QObject::tr("Fatal error"), QObject::tr("Startup failed: %1").arg(QString::fromUtf8(ex.what())));
        return -1;
    } catch (...) {
        fprintf(stderr, "Fatal error during QML startup: unknown exception\n");
        QMessageBox::critical(nullptr, QObject::tr("Fatal error"), QObject::tr("Startup failed: unknown exception"));
        return -2;
    }
#else
    // No UI available in this build configuration
    return 0;
#endif
}