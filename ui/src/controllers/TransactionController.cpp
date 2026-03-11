#include "ui/controllers/TransactionController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"

namespace ui {

namespace {

Transaction::Status toTransactionStatus(int status)
{
    switch (static_cast<Transaction::Status>(status)) {
    case Transaction::Status::Neutral:
    case Transaction::Status::Unverified:
    case Transaction::Status::Verified:
    case Transaction::Status::Completed:
        return static_cast<Transaction::Status>(status);
    }

    return Transaction::Status::Neutral;
}

}

TransactionController::TransactionController(core::controllers::AppStateController *core,
                                             QObject *parent)
    : QObject(parent), core_(core) {}

QString TransactionController::addTransaction(
    const QString &name, const QString &bookingDate, double amount,
    const QString &description, const QString &statementId, int status,
    const QString &actorId, bool allocatable, const QStringList &propertyIds) {
  return controllers::guard::invokeValue<QString>(
      core_, observability::origins::controller::transaction::kAdd, {}, [&]() {
        return QString::fromStdString(core_->addTransaction(
            strings::toStdString(name), strings::toStdString(bookingDate),
            amount, strings::toStdString(description),
            strings::toStdString(statementId), toTransactionStatus(status),
            strings::toStdString(actorId), allocatable,
            strings::toStdList(propertyIds)));
      });
}

void TransactionController::updateTransaction(
    const QString &id, const QString &name, const QString &bookingDate,
    double amount, const QString &description, const QString &statementId,
    int status, const QString &actorId, bool allocatable,
    const QStringList &propertyIds) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::transaction::kUpdate, [&]() {
        core_->updateTransaction(strings::toStdString(id),
                                 strings::toStdString(name),
                                 strings::toStdString(bookingDate), amount,
                                 strings::toStdString(description),
                                 strings::toStdString(statementId),
                                 toTransactionStatus(status),
                                 strings::toStdString(actorId), allocatable,
                                 strings::toStdList(propertyIds));
      });
}

void TransactionController::deleteTransaction(const QString &id) {
  controllers::guard::invokeVoid(
      core_, observability::origins::controller::transaction::kDelete,
      [&]() { core_->deleteTransaction(strings::toStdString(id)); });
}

} // namespace ui
