#include "ui/controllers/ActorController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"

namespace ui {

ActorController::ActorController(core::controllers::AppStateController *core, QObject *parent)
    : QObject(parent), core_(core) {}

QString ActorController::addActor(const QString &name, const QString &type,
                                  const QString &description) {
  return controllers::guard::invokeValue<QString>(
      core_, observability::origins::controller::actor::kAdd, {}, [&]() {
        return QString::fromStdString(core_->addActor(
            strings::toStdString(name), strings::toStdString(type),
            strings::toStdString(description)));
      });
}

void ActorController::updateActor(const QString &id, const QString &name,
                                  const QString &type,
                                  const QString &description) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::actor::kUpdate, [&]() {
        core_->updateActor(strings::toStdString(id), strings::toStdString(name),
                           strings::toStdString(type),
                           strings::toStdString(description));
      });
}

void ActorController::deleteActor(const QString &id) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::actor::kDelete,
      [&]() { core_->deleteActor(strings::toStdString(id)); });
}

} // namespace ui
