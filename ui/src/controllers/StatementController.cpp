#include "ui/controllers/StatementController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"

namespace ui {

StatementController::StatementController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString StatementController::addStatement(const QString& name)
{
    if (!controllers::guard::ensureCore(core_, "ui::StatementController::addStatement")) return {};
    try {
        return QString::fromStdString(core_->addStatement(strings::toStdString(name)));
    } catch (...) {
        controllers::guard::reportException("ui::StatementController::addStatement");
    }
    return {};
}

void StatementController::updateStatement(const QString& id, const QString& name)
{
    if (!controllers::guard::ensureCore(core_, "ui::StatementController::updateStatement")) return;
    try {
        core_->updateStatement(id.toStdString(), strings::toStdString(name));
    } catch (...) {
        controllers::guard::reportException("ui::StatementController::updateStatement");
    }
}

void StatementController::deleteStatement(const QString& id)
{
    if (!controllers::guard::ensureCore(core_, "ui::StatementController::deleteStatement")) return;
    try {
        core_->deleteStatement(id.toStdString());
    } catch (...) {
        controllers::guard::reportException("ui::StatementController::deleteStatement");
    }
}

}
