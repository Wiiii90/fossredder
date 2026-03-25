/**
 * @file ui/src/controllers/ActorController.cpp
 * @brief Implements the UI controller that forwards actor mutations to the application facade.
 */

#include "ui/controllers/ActorController.h"

#include "core/application/AppStateFacade.h"
#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"

namespace ui {

ActorController::ActorController(core::application::AppStateFacade *core, QObject *parent)
    : QObject(parent), core_(core) {}

QString ActorController::addActor(const QString &name, const QString &type,
                                  const QString &description, const QStringList &aliases) {
  return controllers::guard::invokeValue<QString>(
      core_, observability::origins::controller::actor::kAdd, {}, [&]() {
        return QString::fromStdString(core_->addActor(
            strings::toStdString(name), strings::toStdString(type),
            strings::toStdString(description), strings::toStdList(aliases)));
      });
}

void ActorController::updateActor(const QString &id, const QString &name,
                                  const QString &type,
                                  const QString &description,
                                  const QStringList &aliases) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::actor::kUpdate, [&]() {
        core_->updateActor(strings::toStdString(id), strings::toStdString(name),
                           strings::toStdString(type),
                           strings::toStdString(description), strings::toStdList(aliases));
      });
}

void ActorController::deleteActor(const QString &id) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::actor::kDelete,
      [&]() { core_->deleteActor(strings::toStdString(id)); });
}

} // namespace ui
