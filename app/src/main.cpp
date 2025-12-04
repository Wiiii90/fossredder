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
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "ui/qml_models/TransactionGroupModel.h"
#endif

int main(int argc, char* argv[]) {
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
    qmlRegisterType<ui::TransactionGroupModel>("FossRedder", 1, 0, "TransactionGroupModel");
    ui::TransactionGroupModel listModel;
    listModel.loadDemoData();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("transactionModel", &listModel);
    Q_INIT_RESOURCE(qml);

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString runtimeQmlPath = QDir(appDir).filePath("qml");

    // Deterministic: use exactly one QML import path.
    if (QDir(runtimeQmlPath).exists()) {
        engine.addImportPath(runtimeQmlPath);
    } else {
        QString vcpkgInstalled = QString::fromUtf8(qgetenv("VCPKG_INSTALLED_DIR"));
        if (vcpkgInstalled.isEmpty())
            vcpkgInstalled = QDir(appDir).filePath("../vcpkg_installed");
        QString triplet = QString::fromUtf8(qgetenv("VCPKG_TARGET_TRIPLET"));
        if (triplet.isEmpty())
            triplet = QStringLiteral("x64-windows");
        const QString vcpkgQml = QDir(vcpkgInstalled).filePath(triplet + "/Qt6/qml");
        if (QDir(vcpkgQml).exists())
            engine.addImportPath(vcpkgQml);
    }

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    engine.load(url);
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
#else
    return -1;
#endif
}