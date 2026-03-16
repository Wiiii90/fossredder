/**
 * @file app/src/main.cpp
 * @brief Initializes the desktop application and shared runtime infrastructure.
 */

#include "core/utils/Environment.h"
#include "core/models/AppState.h"

using core::domain::AppState;
#include <QApplication>
#include <QMessageBox>
#include <QObject>
#include <QQuickStyle>

#include "persistence/Factory.h"
#include "persistence/AppStateStore.h"
#include "core/constants/CoreDefaults.h"
#include "core/repositories/IConfigRepository.h"
#include "core/storage/StorageManager.h"
#include "core/controllers/AppStateController.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "debug/ErrorReporter.h"
#include "ui/config/Defaults.h"
#include "ui/observability/ErrorCodes.h"

#include <QDir>
#include <filesystem>
#include <cstdio>

namespace {

void ensureParentDirectoryExists(const std::filesystem::path& path, const char* origin)
{
    try {
        if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path());
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, origin, std::current_exception());
    }

}

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
        core::errors::report(core::errors::ErrorSeverity::Info, ui::observability::codes::QtDebug, "app::qtMessageHandler", text, ctx);
        break;
    case QtInfoMsg:
        core::errors::report(core::errors::ErrorSeverity::Info, ui::observability::codes::QtInfo, "app::qtMessageHandler", text, ctx);
        break;
    case QtWarningMsg:
        core::errors::report(core::errors::ErrorSeverity::Warning, ui::observability::codes::QtWarning, "app::qtMessageHandler", text, ctx);
        break;
    case QtCriticalMsg:
        core::errors::report(core::errors::ErrorSeverity::Error, ui::observability::codes::QtCritical, "app::qtMessageHandler", text, ctx);
        break;
    case QtFatalMsg:
        core::errors::report(core::errors::ErrorSeverity::Critical, ui::observability::codes::QtFatal, "app::qtMessageHandler", text, ctx);
        abort();
    }
}

}

#ifdef USE_QML
/**
 * @brief Start the QML application UI.
 *
 * Implemented in `main_qml.cpp`. Only available when built with USE_QML.
 */
extern int startQmlApp(QApplication& app, core::controllers::AppStateController& appStateCtrl);
#endif

int main(int argc, char* argv[]) {
    // Install global Qt message handler early so startup logs are captured
    const auto previousQtMessageHandler = qInstallMessageHandler(qtMessageHandler);

    // Load runtime environment from .env if present
    env::load_dotenv(".env", false);

    // Ensure Qt Quick Controls uses a non-native style that supports customization
    // Call before creating the QApplication/QGuiApplication
    QQuickStyle::setStyle(core::constants::runtime::kQtStyle.data());

    // Create the Qt application (manages event loop and GUI resources)
    QApplication app(argc, argv);
    app.setStyle(core::constants::runtime::kQtStyle.data());
    app.setOrganizationName(ui::config::kSettingsOrganizationName);
    app.setApplicationName(ui::config::kSettingsApplicationName);

    // Setup storage manager and controller (manages application state files)
    const std::filesystem::path appDataRoot = std::filesystem::path(QDir::homePath().toStdString())
        / std::string(core::constants::runtime::kAppDataDirectoryName);
    core::storage::StorageManager sm(appDataRoot.string());
    auto smPtr = std::make_unique<core::storage::StorageManager>(std::move(sm));
    core::controllers::AppStateController appStateCtrl(std::move(smPtr));

    auto errorReporter = debug::createDefaultErrorReporter();
    core::errors::setGlobalErrorReporter(errorReporter);
    appStateCtrl.setErrorReporter(errorReporter);

    // Initialize configuration repository (uses persistence factory)
    // Use a per-user path so the installed app does not attempt to write into Program Files.
    const std::filesystem::path cfgDbPath = appDataRoot / std::string(core::constants::runtime::kDatabaseFileName);
    ensureParentDirectoryExists(cfgDbPath, "app::main::createConfigDirectory");

    std::shared_ptr<IConfigRepository> cfgRepo;
    try {
        auto cfgDb = createSqliteDb(cfgDbPath.string());
        cfgRepo = createSqliteConfigRepository(cfgDb);
    } catch (const std::exception& ex) {
        core::errors::report(
            core::errors::ErrorSeverity::Warning,
            core::errors::codes::ConfigDbOpenFailed,
            "app::main::openConfigDb",
            std::string("failed to open config DB '") + cfgDbPath.string() + "': " + ex.what(),
            {{"path", cfgDbPath.string()}}
        );
        cfgRepo = nullptr;
    }

    appStateCtrl.setRepoFactory([&](const std::string& dbPath) {
        ensureParentDirectoryExists(std::filesystem::path(dbPath), "app::main::setRepoFactory::create_directories");
        auto db = createSqliteDb(dbPath);
        return createSqliteRepositoryBundle(db, errorReporter);
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
    if (appStateCtrl.currentPath().empty() && appStateCtrl.state().empty()) {
        appStateCtrl.newFile((appDataRoot / std::string(core::constants::runtime::kDatabaseFileName)).string());
    }

    // Ensure Qt finds deployed plugins and QML modules next to the executable
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());

#ifdef USE_QML
    // Delegate to QML-specific startup
    try {
        const int exitCode = startQmlApp(app, appStateCtrl);
        qInstallMessageHandler(previousQtMessageHandler);
        appStateCtrl.setErrorReporter({});
        core::errors::setGlobalErrorReporter({});
        return exitCode;
    } catch (const std::exception& ex) {
        qInstallMessageHandler(previousQtMessageHandler);
        core::errors::reportException(core::errors::ErrorSeverity::Critical, "app::main::startQmlApp", std::current_exception());
        QMessageBox::critical(nullptr, QObject::tr("Fatal error"), QObject::tr("Startup failed: %1").arg(QString::fromUtf8(ex.what())));
        appStateCtrl.setErrorReporter({});
        core::errors::setGlobalErrorReporter({});
        return -1;
    } catch (...) {
        qInstallMessageHandler(previousQtMessageHandler);
        core::errors::reportException(core::errors::ErrorSeverity::Critical, "app::main::startQmlAppUnknown", std::current_exception());
        QMessageBox::critical(nullptr, QObject::tr("Fatal error"), QObject::tr("Startup failed: unknown exception"));
        appStateCtrl.setErrorReporter({});
        core::errors::setGlobalErrorReporter({});
        return -2;
    }
#else
    // No UI available in this build configuration
    qInstallMessageHandler(previousQtMessageHandler);
    return 0;
#endif
}