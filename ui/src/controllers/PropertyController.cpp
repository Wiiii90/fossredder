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
    return controllers::guard::invokeValue<QString>(core_, "ui::PropertyController::addProperty", {}, [&]() {
        return QString::fromStdString(core_->addProperty(strings::toStdString(name), strings::toStdString(address), strings::toStdString(description)));
    });
}

void PropertyController::updateProperty(const QString& id, const QString& name, const QString& address, const QString& description)
{
    controllers::guard::invokeVoid(core_, "ui::PropertyController::updateProperty", [&]() {
        core_->updateProperty(id.toStdString(), strings::toStdString(name), strings::toStdString(address), strings::toStdString(description));
    });
}

void PropertyController::deleteProperty(const QString& id)
{
    controllers::guard::invokeVoid(core_, "ui::PropertyController::deleteProperty", [&]() {
        core_->deleteProperty(id.toStdString());
    });
}

}
