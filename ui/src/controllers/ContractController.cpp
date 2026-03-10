#include "ui/controllers/ContractController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"

#include "core/models/Contract.h"

#include <unordered_set>

namespace ui {

ContractController::ContractController(AppStateController *core,
                                       QObject *parent)
    : QObject(parent), core_(core) {}

QString ContractController::addContract(const QString &name,
                                        const QString &type,
                                        const QString &description,
                                        const QStringList &actorIds,
                                        const QStringList &propertyIds) {
  return controllers::guard::invokeValue<QString>(
      core_, observability::origins::controller::contract::kAdd, {}, [&]() {
        return QString::fromStdString(core_->addContract(
            strings::toStdString(name), strings::toStdString(type),
            strings::toStdString(description), strings::toStdList(actorIds),
            strings::toStdList(propertyIds)));
      });
}

void ContractController::updateContract(const QString &id, const QString &name,
                                        const QString &type,
                                        const QString &description,
                                        const QStringList &actorIds,
                                        const QStringList &propertyIds) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::contract::kUpdate, [&]() {
        core_->updateContract(
            strings::toStdString(id), strings::toStdString(name),
            strings::toStdString(type), strings::toStdString(description),
            strings::toStdList(actorIds), strings::toStdList(propertyIds));
      });
}

void ContractController::deleteContract(const QString &id) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::contract::kDelete,
      [&]() { core_->deleteContract(strings::toStdString(id)); });
}

QStringList ContractController::getContractTypes() const {
  return controllers::guard::invokeValue<QStringList>(
      core_, observability::origins::controller::contract::kGetTypes, {},
      [&]() {
        QStringList out;
        std::unordered_set<std::string> seen;
        for (const auto &c : core_->state().contracts) {
          if (!c)
            continue;
          const std::string type = c->type;
          if (type.empty())
            continue;
          if (seen.find(type) != seen.end())
            continue;
          seen.insert(type);
          out.push_back(QString::fromStdString(type));
        }
        return out;
      });
}

} // namespace ui
