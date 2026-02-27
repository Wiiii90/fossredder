#include "ui/controllers/ActorController.h"

#include <QUuid>
#include <algorithm>
#include <memory>

#include "ui/controllers/ControllerStrings.h"
#include "core/models/Actor.h"

namespace ui {

ActorController::ActorController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString ActorController::addActor(const QString& name, const QString& type, const QString& description, const QStringList& aliases)
{
    if (!core_) return {};
    auto actor = std::make_shared<Actor>();
    actor->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    actor->name = strings::toStdString(name);
    actor->type = strings::toStdString(type);
    actor->description = strings::toStdString(description);
    actor->aliases = strings::toStdListTrimmed(aliases);
    core_->mutableState().actors.push_back(actor);
    core_->notifyState();
    core_->commit();
    return QString::fromStdString(actor->id);
}

void ActorController::updateActor(const QString& id, const QString& name, const QString& type, const QString& description, const QStringList& aliases)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    for (auto& a : core_->mutableState().actors) {
        if (!a || a->id != sid) continue;
        a->name = strings::toStdString(name);
        a->type = strings::toStdString(type);
        a->description = strings::toStdString(description);
        a->aliases = strings::toStdListTrimmed(aliases);
        core_->notifyState();
        core_->commit();
        return;
    }
}

void ActorController::deleteActor(const QString& id)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    auto& v = core_->mutableState().actors;
    v.erase(std::remove_if(v.begin(), v.end(), [&](const auto& a) { return a && a->id == sid; }), v.end());
    core_->notifyState();
    core_->commit();
}

}
