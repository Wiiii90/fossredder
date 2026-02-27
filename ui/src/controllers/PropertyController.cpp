#include "ui/controllers/PropertyController.h"

#include <QUuid>
#include <algorithm>
#include <memory>

#include "core/models/Property.h"

namespace {
std::string q2s(const QString& s)
{
    const auto u8 = s.toUtf8();
    return std::string(u8.constData(), static_cast<size_t>(u8.size()));
}
}

PropertyController::PropertyController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString PropertyController::addProperty(const QString& name, const QString& address, const QString& description)
{
    if (!core_) return {};
    auto p = std::make_shared<Property>();
    p->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    p->name = q2s(name);
    p->address = q2s(address);
    p->description = q2s(description);
    core_->mutableState().properties.push_back(p);
    core_->notifyState();
    return QString::fromStdString(p->id);
}

void PropertyController::updateProperty(const QString& id, const QString& name, const QString& address, const QString& description)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    for (auto& p : core_->mutableState().properties) {
        if (!p || p->id != sid) continue;
        p->name = q2s(name);
        p->address = q2s(address);
        p->description = q2s(description);
        core_->notifyState();
        return;
    }
}

void PropertyController::deleteProperty(const QString& id)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    auto& v = core_->mutableState().properties;
    v.erase(std::remove_if(v.begin(), v.end(), [&](const auto& p) { return p && p->id == sid; }), v.end());
    core_->notifyState();
}
