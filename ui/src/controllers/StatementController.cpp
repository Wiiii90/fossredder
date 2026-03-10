#include "ui/controllers/StatementController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"

namespace ui {

StatementController::StatementController(AppStateController *core,
                                         QObject *parent)
    : QObject(parent), core_(core) {}

QString StatementController::addStatement(const QString &name) {
  return controllers::guard::invokeValue<QString>(
      core_, observability::origins::controller::statement::kAdd, {}, [&]() {
        return QString::fromStdString(
            core_->addStatement(strings::toStdString(name)));
      });
}

void StatementController::updateStatement(const QString &id,
                                          const QString &name) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::statement::kUpdate, [&]() {
        core_->updateStatement(strings::toStdString(id),
                               strings::toStdString(name));
      });
}

void StatementController::deleteStatement(const QString &id) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::statement::kDelete,
      [&]() { core_->deleteStatement(strings::toStdString(id)); });
}

} // namespace ui
