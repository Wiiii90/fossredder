#include "ui/controllers/ContractController.h"

#include <QUuid>
#include <algorithm>
#include <memory>

#include "ui/controllers/ControllerStrings.h"
#include "core/models/Contract.h"

namespace ui {

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
    c->name = strings::toStdString(name);
    c->type = strings::toStdString(type);
    c->description = strings::toStdString(description);
    c->actorIds = strings::toStdList(actorIds);
    c->propertyIds = strings::toStdList(propertyIds);
    core_->mutableState().contracts.push_back(c);
    core_->notifyState();
    core_->commit();
    return QString::fromStdString(c->id);
}

void ContractController::updateContract(const QString& id, const QString& name, const QString& type, const QString& description,
                                        const QStringList& actorIds, const QStringList& propertyIds)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    for (auto& c : core_->mutableState().contracts) {
        if (!c || c->id != sid) continue;
        c->name = strings::toStdString(name);
        c->type = strings::toStdString(type);
        c->description = strings::toStdString(description);
        c->actorIds = strings::toStdList(actorIds);
        c->propertyIds = strings::toStdList(propertyIds);
        core_->notifyState();
        core_->commit();
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
    core_->commit();
}

}
