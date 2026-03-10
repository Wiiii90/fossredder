#include "ui/controllers/PropertyController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"

namespace ui {

PropertyController::PropertyController(AppStateController *core,
                                       QObject *parent)
    : QObject(parent), core_(core) {}

QString PropertyController::addProperty(const QString &name,
                                        const QString &address,
                                        const QString &description) {
  return controllers::guard::invokeValue<QString>(
      core_, observability::origins::controller::property::kAdd, {}, [&]() {
        return QString::fromStdString(core_->addProperty(
            strings::toStdString(name), strings::toStdString(address),
            strings::toStdString(description)));
      });
}

void PropertyController::updateProperty(const QString &id, const QString &name,
                                        const QString &address,
                                        const QString &description) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::property::kUpdate, [&]() {
        core_->updateProperty(
            strings::toStdString(id), strings::toStdString(name),
            strings::toStdString(address), strings::toStdString(description));
      });
}

void PropertyController::deleteProperty(const QString &id) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::property::kDelete,
      [&]() { core_->deleteProperty(strings::toStdString(id)); });
}

} // namespace ui
