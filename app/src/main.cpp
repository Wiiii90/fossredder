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
#include "core/errors/DebuggerErrorReporter.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "debug/FileDebugger.h"

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
    const std::string text = std::string(localMsg.constData()) + " (" + file + ":" + std::to_string(context.line) + ", " + function + ")";
    const core::errors::ErrorContext ctx = {
        {"file", file},
        {"line", std::to_string(context.line)},
        {"function", function}
    };
    switch (type) {
    case QtDebugMsg:
        core::errors::report(core::errors::ErrorSeverity::Info, core::errors::codes::QtDebug, "app::qtMessageHandler", text, ctx);
        break;
    case QtInfoMsg:
        core::errors::report(core::errors::ErrorSeverity::Info, core::errors::codes::QtInfo, "app::qtMessageHandler", text, ctx);
        break;
    case QtWarningMsg:
        core::errors::report(core::errors::ErrorSeverity::Warning, core::errors::codes::QtWarning, "app::qtMessageHandler", text, ctx);
        break;
    case QtCriticalMsg:
        core::errors::report(core::errors::ErrorSeverity::Error, core::errors::codes::QtCritical, "app::qtMessageHandler", text, ctx);
        break;
    case QtFatalMsg:
        core::errors::report(core::errors::ErrorSeverity::Critical, core::errors::codes::QtFatal, "app::qtMessageHandler", text, ctx);
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

    auto errorReporter = std::make_shared<core::errors::DebuggerErrorReporter>(
        std::make_shared<FileDebugger>("", "errors"));
    core::errors::setGlobalErrorReporter(errorReporter);
    appStateCtrl.setErrorReporter(errorReporter);

    // Initialize configuration repository (uses persistence factory)
    // Use a per-user path so the installed app does not attempt to write into Program Files.
    std::string cfgDbPath = (std::filesystem::path(appDataRoot) / "fossredder.db").string();

    // ensure parent directory exists
    try {
        std::filesystem::path p(cfgDbPath);
        if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());
    } catch (const std::exception& ex) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "app::main::createConfigDirectory", std::current_exception());
    }

    std::shared_ptr<IConfigRepository> cfgRepo;
    try {
        auto cfgDb = createSqliteDb(cfgDbPath);
        cfgRepo = createSqliteConfigRepository(cfgDb);
    } catch (const std::exception& ex) {
        core::errors::report(
            core::errors::ErrorSeverity::Warning,
            core::errors::codes::ConfigDbOpenFailed,
            "app::main::openConfigDb",
            std::string("failed to open config DB '") + cfgDbPath + "': " + ex.what(),
            {{"path", cfgDbPath}}
        );
        cfgRepo = nullptr;
    }

    appStateCtrl.setRepoFactory([&](const std::string& dbPath) {
        // ensure directory exists for requested dbPath
        try {
            std::filesystem::path p(dbPath);
            if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());
        } catch (...) {
            core::errors::reportException(core::errors::ErrorSeverity::Warning, "app::main::setRepoFactory::create_directories", std::current_exception());
        }
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
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "app::main::openLatest", std::current_exception());
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
            core::errors::reportException(core::errors::ErrorSeverity::Warning, "app::main::getDefaultConfig", std::current_exception());
        }
    }

    // Ensure Qt finds deployed plugins and QML modules next to the executable
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());

#ifdef USE_QML
    // Delegate to QML-specific startup
    try {
        return startQmlApp(app, appStateCtrl);
    } catch (const std::exception& ex) {
        core::errors::reportException(core::errors::ErrorSeverity::Critical, "app::main::startQmlApp", std::current_exception());
        QMessageBox::critical(nullptr, QObject::tr("Fatal error"), QObject::tr("Startup failed: %1").arg(QString::fromUtf8(ex.what())));
        return -1;
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Critical, "app::main::startQmlAppUnknown", std::current_exception());
        QMessageBox::critical(nullptr, QObject::tr("Fatal error"), QObject::tr("Startup failed: unknown exception"));
        return -2;
    }
#else
    // No UI available in this build configuration
    return 0;
#endif
}