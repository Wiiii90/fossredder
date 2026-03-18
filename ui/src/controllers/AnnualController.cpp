/**
 * @file ui/src/controllers/AnnualController.cpp
 * @brief Implements the UI controller that forwards annual aggregate creation to the application facade.
 */

#include "ui/controllers/AnnualController.h"

#include "core/application/AppStateFacade.h"
#include "ui/controllers/ControllerGuard.h"
#include "ui/observability/Origins.h"

namespace ui {

AnnualController::AnnualController(core::application::AppStateFacade *core, QObject *parent)
    : QObject(parent), core_(core) {}

QString AnnualController::addAnnual(int year) {
  return controllers::guard::invokeValue<QString>(
      core_, observability::origins::controller::annual::kAdd, {},
      [&]() { return QString::fromStdString(core_->addAnnual(year)); });
}

} // namespace ui
