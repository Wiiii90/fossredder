#include "app/pch.h"
#include "core/utils/Environment.h"
#include <QApplication>

#include "core/repositories/IConfigRepository.h"
#include "persistence/Factory.h"
#include "core/models/Config.h"
#include "core/managers/ConfigManager.h"
#include "core/controllers/StatementController.h"

#include <QFile>
#include <QDir>
#include <QDebug>

#ifdef USE_QML
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickView>
#include <QQuickWindow>
#include "ui/qml_models/TransactionGroupModel.h"
#endif

int main(int argc, char* argv[]) {
    bool loaded = env::load_dotenv(".env", false);
    (void)loaded;

    std::string dbPath = "fossredder.db";
    auto actorRepo = createSqliteActorRepository(dbPath);
    auto propRepo = createSqlitePropertyRepository(dbPath);
    auto stmtRepo = createSqliteStatementRepository(dbPath);
    auto cfgRepo = createSqliteConfigRepository(dbPath);
    auto txRepo = createSqliteTransactionRepository(dbPath);
    auto bgRepo = createSqliteBookingGroupRepository(dbPath);

    ConfigManager cfgMgr;
    auto def = cfgRepo->getDefaultConfig();
    if (def) cfgMgr.setConfig(*def);

    QApplication app(argc, argv);
    app.setStyle("Fusion");

#ifdef USE_QML
    // --- Debug: print environment to verify PATH and QT_DEBUG_PLUGINS are visible here
    QString appDir = QCoreApplication::applicationDirPath();
    qDebug() << "[ENV CHECK] Application dir:" << appDir;
    QByteArray pathEnv = qgetenv("PATH");
    QByteArray qtDebugEnv = qgetenv("QT_DEBUG_PLUGINS");
    QByteArray vcpkgEnv = qgetenv("VCPKG_INSTALLED_DIR");
    qDebug() << "[ENV CHECK] PATH (prefix):" << QString(pathEnv).left(512);
    qDebug() << "[ENV CHECK] QT_DEBUG_PLUGINS:" << qtDebugEnv;
    qDebug() << "[ENV CHECK] VCPKG_INSTALLED_DIR env:" << vcpkgEnv;

    // check expected vcpkg debug bin and plugin DLL presence
    // appDir is .../build/x64-Debug-vcpkg/bin -> vcpkg_installed is at project root P:/fossredder/vcpkg_installed
    // go up two levels from bin to repository root, unless VCPKG_INSTALLED_DIR env is set
    QString vcpkgDebugBin;
    if (!vcpkgEnv.isEmpty()) {
        vcpkgDebugBin = QDir(QString::fromUtf8(vcpkgEnv)).filePath("x64-windows/debug/bin");
    } else {
        // appDir is .../build/x64-Debug-vcpkg/bin -> go up three levels to reach repo root
        vcpkgDebugBin = QDir(appDir).filePath("..\\..\\..\\vcpkg_installed\\x64-windows\\debug\\bin");
    }
    QString pluginDll = QDir(appDir).filePath("qml/QtQuick/qtquick2plugind.dll");
    qDebug() << "[ENV CHECK] vcpkg debug bin exists:" << QDir(vcpkgDebugBin).exists() << vcpkgDebugBin;
    qDebug() << "[ENV CHECK] qtquick2plugind.dll exists in runtime qml path:" << QFile::exists(pluginDll) << pluginDll;
    // Also check in vcpkg debug location
    QString pluginInVcpkg = QDir(vcpkgDebugBin).filePath("../Qt6/qml/QtQuick/qtquick2plugind.dll");
    qDebug() << "[ENV CHECK] qtquick2plugind.dll exists in vcpkg debug qml:" << QFile::exists(pluginInVcpkg) << pluginInVcpkg;

    // Runtime PATH/VCPKG fallback removed; rely on CMake and RunDebug.bat for debug environment

    // Register and populate model
    qmlRegisterType<ui::TransactionGroupModel>("FossRedder", 1, 0, "TransactionGroupModel");
    ui::TransactionGroupModel listModel;
    listModel.loadDemoData();

    // Try to load QML POC
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("transactionModel", &listModel);
    Q_INIT_RESOURCE(qml);

    // Ensure engine can find QML modules copied to the runtime directory (bin/qml)
    QString runtimeQmlPath = QDir(appDir).filePath("qml");
    engine.addImportPath(runtimeQmlPath);
    engine.addImportPath(runtimeQmlPath + "/QtQuick");
    engine.addImportPath(runtimeQmlPath + "/QtQuick/Controls.2");

    // Also try common vcpkg_installed locations inside project (release and debug)
    QStringList candidates;
    candidates << QDir::cleanPath(QDir(appDir).filePath("..\\vcpkg_installed\\x64-windows\\Qt6\\qml"))
               << QDir::cleanPath(QDir(appDir).filePath("..\\vcpkg_installed\\x64-windows\\debug\\Qt6\\qml"))
               << QDir::cleanPath("C:/coding/fossredder/vcpkg_installed/x64-windows/Qt6/qml")
               << QDir::cleanPath("C:/coding/fossredder/vcpkg_installed/x64-windows/debug/Qt6/qml");

    for (const QString &p : candidates) {
        if (QDir(p).exists()) {
            engine.addImportPath(p);
            engine.addImportPath(p + "/QtQuick");
            engine.addImportPath(p + "/QtQuick/Controls");
            engine.addImportPath(p + "/QtQuick/Controls/Basic");
            qDebug() << "Added QML import path:" << p;
        }
    }

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    engine.load(url);
    if (engine.rootObjects().isEmpty()) {
        qWarning() << "Failed to load QML UI";
        return -1;
    }

    return app.exec();
#else
    qWarning() << "QML support not enabled at build time";
    return -1;
#endif
}