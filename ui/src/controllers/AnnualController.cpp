#include "ui/controllers/AnnualController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"

namespace ui {

AnnualController::AnnualController(core::controllers::AppStateController *core, QObject *parent)
    : QObject(parent), core_(core) {}

QString AnnualController::addAnnual(int year) {
  return controllers::guard::invokeValue<QString>(
      core_, observability::origins::controller::annual::kAdd, {},
      [&]() { return QString::fromStdString(core_->addAnnual(year)); });
}

void AnnualController::updateAnnual(const QString &id, int year) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::annual::kUpdate,
      [&]() { core_->updateAnnual(strings::toStdString(id), year); });
}

void AnnualController::deleteAnnual(const QString &id) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::annual::kDelete,
      [&]() { core_->deleteAnnual(strings::toStdString(id)); });
}

} // namespace ui
