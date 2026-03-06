#include "ui/controllers/ActorController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"

namespace ui {

ActorController::ActorController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString ActorController::addActor(const QString& name, const QString& type, const QString& description, const QStringList& aliases)
{
    return controllers::guard::invokeValue<QString>(core_, "ui::ActorController::addActor", {}, [&]() {
        return QString::fromStdString(core_->addActor(strings::toStdString(name), strings::toStdString(type), strings::toStdString(description), strings::toStdListTrimmed(aliases)));
    });
}

void ActorController::updateActor(const QString& id, const QString& name, const QString& type, const QString& description, const QStringList& aliases)
{
    controllers::guard::invokeVoid(core_, "ui::ActorController::updateActor", [&]() {
        core_->updateActor(id.toStdString(), strings::toStdString(name), strings::toStdString(type), strings::toStdString(description), strings::toStdListTrimmed(aliases));
    });
}

void ActorController::deleteActor(const QString& id)
{
    controllers::guard::invokeVoid(core_, "ui::ActorController::deleteActor", [&]() {
        core_->deleteActor(id.toStdString());
    });
}

}
