#include "app/pch.h"
#include "core/utils/Environment.h"
#include <QApplication>

#include "core/repositories/IConfigRepository.h"
#include "persistence/Factory.h"
#include "core/models/Config.h"
#include "core/managers/ConfigManager.h"
#include "core/controllers/StatementController.h"
#include "ui/windows/MainWindow.h"

#include <QFile>
#include <QDir>
#include <QDebug>

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

    QString qss;

    // Prefer loading stylesheet from the filesystem (for development/hot-reload).
    // If not found, fall back to the embedded resource compiled into the binary.
    QDir dir(QCoreApplication::applicationDirPath());
    bool found = false;
    QStringList candidatesRel = {
        "ui/res/styles/app.qss",
        "ui/resources/styles/app.qss",
        "ui/styles/app.qss",
        "resources/styles/app.qss",
        "styles/app.qss",
        "ui/resources/app.qss",
    };
    for (int depth = 0; depth < 8 && !found; ++depth) {
        for (const QString& rel : candidatesRel) {
            QString p = dir.filePath(rel);
            QFile ff(p);
            if (ff.open(QFile::ReadOnly | QFile::Text)) {
                qss = QString::fromUtf8(ff.readAll());
                ff.close();
                qDebug() << "Loaded application stylesheet from file:" << p;
                found = true;
                break;
            }
        }
        if (!found) {
            if (!dir.cdUp()) break;
        }
    }

    if (!found) {
        // No external stylesheet found — try embedded resource
        Q_INIT_RESOURCE(resources);
        QFile f(":/styles/app.qss");
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            qss = QString::fromUtf8(f.readAll());
            f.close();
            qDebug() << "Loaded application stylesheet from resource";
        }
    }

    if (!qss.isEmpty()) {
        app.setStyleSheet(qss);
        qDebug() << "Applied application stylesheet (length):" << qss.size();
    } else {
        qDebug() << "No application stylesheet loaded";
    }

    ui::MainWindow mainWindow(nullptr, actorRepo, propRepo, stmtRepo, cfgRepo, txRepo, bgRepo);
    mainWindow.show();
    return app.exec();
}