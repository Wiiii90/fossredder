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
    if (!controllers::guard::ensureCore(core_, "ui::ActorController::addActor")) return {};
    try {
        return QString::fromStdString(core_->addActor(strings::toStdString(name), strings::toStdString(type), strings::toStdString(description), strings::toStdListTrimmed(aliases)));
    } catch (...) {
        controllers::guard::reportException("ui::ActorController::addActor");
    }
    return {};
}

void ActorController::updateActor(const QString& id, const QString& name, const QString& type, const QString& description, const QStringList& aliases)
{
    if (!controllers::guard::ensureCore(core_, "ui::ActorController::updateActor")) return;
    try {
        core_->updateActor(id.toStdString(), strings::toStdString(name), strings::toStdString(type), strings::toStdString(description), strings::toStdListTrimmed(aliases));
    } catch (...) {
        controllers::guard::reportException("ui::ActorController::updateActor");
    }
}

void ActorController::deleteActor(const QString& id)
{
    if (!controllers::guard::ensureCore(core_, "ui::ActorController::deleteActor")) return;
    try {
        core_->deleteActor(id.toStdString());
    } catch (...) {
        controllers::guard::reportException("ui::ActorController::deleteActor");
    }
}

}
