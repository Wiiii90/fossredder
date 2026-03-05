#include "ui/controllers/PropertyController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"

namespace ui {

PropertyController::PropertyController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString PropertyController::addProperty(const QString& name, const QString& address, const QString& description)
{
    if (!controllers::guard::ensureCore(core_, "ui::PropertyController::addProperty")) return {};
    try {
        return QString::fromStdString(core_->addProperty(strings::toStdString(name), strings::toStdString(address), strings::toStdString(description)));
    } catch (...) {
        controllers::guard::reportException("ui::PropertyController::addProperty");
    }
    return {};
}

void PropertyController::updateProperty(const QString& id, const QString& name, const QString& address, const QString& description)
{
    if (!controllers::guard::ensureCore(core_, "ui::PropertyController::updateProperty")) return;
    try {
        core_->updateProperty(id.toStdString(), strings::toStdString(name), strings::toStdString(address), strings::toStdString(description));
    } catch (...) {
        controllers::guard::reportException("ui::PropertyController::updateProperty");
    }
}

void PropertyController::deleteProperty(const QString& id)
{
    if (!controllers::guard::ensureCore(core_, "ui::PropertyController::deleteProperty")) return;
    try {
        core_->deleteProperty(id.toStdString());
    } catch (...) {
        controllers::guard::reportException("ui::PropertyController::deleteProperty");
    }
}

}
