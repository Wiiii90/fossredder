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
    return controllers::guard::invokeValue<QString>(core_, "ui::StatementController::addStatement", {}, [&]() {
        return QString::fromStdString(core_->addStatement(strings::toStdString(name)));
    });
}

void StatementController::updateStatement(const QString& id, const QString& name)
{
    controllers::guard::invokeVoid(core_, "ui::StatementController::updateStatement", [&]() {
        core_->updateStatement(id.toStdString(), strings::toStdString(name));
    });
}

void StatementController::deleteStatement(const QString& id)
{
    controllers::guard::invokeVoid(core_, "ui::StatementController::deleteStatement", [&]() {
        core_->deleteStatement(id.toStdString());
    });
}

}
