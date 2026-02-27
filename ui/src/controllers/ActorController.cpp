#include "ui/controllers/ActorController.h"

#include <QUuid>
#include <algorithm>
#include <memory>

#include "core/models/Actor.h"

namespace {
std::string q2s(const QString& s)
{
    const auto u8 = s.toUtf8();
    return std::string(u8.constData(), static_cast<size_t>(u8.size()));
}

std::vector<std::string> toStdAliases(const QStringList& aliases)
{
    std::vector<std::string> out;
    out.reserve(static_cast<size_t>(aliases.size()));
    for (const auto& a : aliases) {
        const auto t = a.trimmed();
        if (t.isEmpty()) continue;
        out.push_back(t.toStdString());
    }
    return out;
}
}

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
    actor->name = q2s(name);
    actor->type = q2s(type);
    actor->description = q2s(description);
    actor->aliases = toStdAliases(aliases);
    core_->mutableState().actors.push_back(actor);
    core_->notifyState();
    return QString::fromStdString(actor->id);
}

void ActorController::updateActor(const QString& id, const QString& name, const QString& type, const QString& description, const QStringList& aliases)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    for (auto& a : core_->mutableState().actors) {
        if (!a || a->id != sid) continue;
        a->name = q2s(name);
        a->type = q2s(type);
        a->description = q2s(description);
        a->aliases = toStdAliases(aliases);
        core_->notifyState();
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
}
