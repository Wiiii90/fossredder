#include "app/pch.h"
#include "core/utils/Environment.h"
#include <QApplication>

#include "core/repositories/IConfigRepository.h"
#include "persistence/Factory.h"
#include "core/models/Config.h"
#include "core/managers/ConfigManager.h"
#include "core/controllers/StatementController.h"
#include "ui/windows/MainWindow.h"

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
    ui::MainWindow mainWindow(nullptr, actorRepo, propRepo, stmtRepo, cfgRepo, txRepo, bgRepo);
    mainWindow.show();
    return app.exec();
}