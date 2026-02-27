#include "ui/controllers/PropertyController.h"

#include <QUuid>
#include <algorithm>
#include <memory>

#include "ui/controllers/ControllerStrings.h"
#include "core/models/Property.h"

namespace ui {

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
    p->name = strings::toStdString(name);
    p->address = strings::toStdString(address);
    p->description = strings::toStdString(description);
    core_->mutableState().properties.push_back(p);
    core_->notifyState();
    core_->commit();
    return QString::fromStdString(p->id);
}

void PropertyController::updateProperty(const QString& id, const QString& name, const QString& address, const QString& description)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    for (auto& p : core_->mutableState().properties) {
        if (!p || p->id != sid) continue;
        p->name = strings::toStdString(name);
        p->address = strings::toStdString(address);
        p->description = strings::toStdString(description);
        core_->notifyState();
        core_->commit();
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
    core_->commit();
}

}
