/**
 * @file ui/src/controllers/ContractController.cpp
 * @brief Implements the UI controller that forwards contract mutations to the application facade.
 */

#include "ui/controllers/ContractController.h"

#include "core/application/AppStateFacade.h"
#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"

namespace ui {

ContractController::ContractController(core::application::AppStateFacade *core,
                                       QObject *parent)
    : QObject(parent), core_(core) {}

QString ContractController::addContract(const QString &name,
                                        const QString &type,
                                        const QString &description,
                                        const QStringList &actorIds,
                                        const QStringList &propertyIds,
                                        const QStringList &aliases) {
  return controllers::guard::invokeValue<QString>(
      core_, observability::origins::controller::contract::kAdd, {}, [&]() {
        return QString::fromStdString(core_->addContract(
            strings::toStdString(name), strings::toStdString(type),
            strings::toStdString(description), strings::toStdList(actorIds),
            strings::toStdList(propertyIds), strings::toStdList(aliases)));
      });
}

void ContractController::updateContract(const QString &id, const QString &name,
                                        const QString &type,
                                        const QString &description,
                                        const QStringList &actorIds,
                                         const QStringList &propertyIds,
                                         const QStringList &aliases) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::contract::kUpdate, [&]() {
        core_->updateContract(
            strings::toStdString(id), strings::toStdString(name),
            strings::toStdString(type), strings::toStdString(description),
            strings::toStdList(actorIds), strings::toStdList(propertyIds), strings::toStdList(aliases));
      });
}

void ContractController::deleteContract(const QString &id) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::contract::kDelete,
      [&]() { core_->deleteContract(strings::toStdString(id)); });
}

} // namespace ui
