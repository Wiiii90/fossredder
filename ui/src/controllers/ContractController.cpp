#include "ui/controllers/ContractController.h"

#include <QUuid>
#include <algorithm>
#include <memory>

#include "core/models/Contract.h"

namespace {
std::string q2s(const QString& s)
{
    const auto u8 = s.toUtf8();
    return std::string(u8.constData(), static_cast<size_t>(u8.size()));
}

std::vector<std::string> toStdIds(const QStringList& ids)
{
    std::vector<std::string> out;
    out.reserve(static_cast<size_t>(ids.size()));
    for (const auto& id : ids) out.push_back(id.toStdString());
    return out;
}
}

ContractController::ContractController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString ContractController::addContract(const QString& name, const QString& type, const QString& description,
                                        const QStringList& actorIds, const QStringList& propertyIds)
{
    if (!core_) return {};
    auto c = std::make_shared<Contract>();
    c->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    c->name = q2s(name);
    c->type = q2s(type);
    c->description = q2s(description);
    c->actorIds = toStdIds(actorIds);
    c->propertyIds = toStdIds(propertyIds);
    core_->mutableState().contracts.push_back(c);
    core_->notifyState();
    return QString::fromStdString(c->id);
}

void ContractController::updateContract(const QString& id, const QString& name, const QString& type, const QString& description,
                                        const QStringList& actorIds, const QStringList& propertyIds)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    for (auto& c : core_->mutableState().contracts) {
        if (!c || c->id != sid) continue;
        c->name = q2s(name);
        c->type = q2s(type);
        c->description = q2s(description);
        c->actorIds = toStdIds(actorIds);
        c->propertyIds = toStdIds(propertyIds);
        core_->notifyState();
        return;
    }
}

void ContractController::deleteContract(const QString& id)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    auto& v = core_->mutableState().contracts;
    v.erase(std::remove_if(v.begin(), v.end(), [&](const auto& c) { return c && c->id == sid; }), v.end());
    core_->notifyState();
}
