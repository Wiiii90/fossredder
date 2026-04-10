/**
 * @file ui/src/controllers/TransactionController.cpp
 * @brief Implements the UI controller that forwards transaction mutations to the application facade.
 */

#include "ui/controllers/TransactionController.h"

#include <algorithm>

#include "core/application/AppStateFacade.h"
#include "ui/observability/Origins.h"
#include "ui/payload/EntityPayloadMapper.h"
#include "ui/util/CoreFacadeGuard.h"
#include "ui/util/StringConversions.h"

namespace ui {

namespace {

/** @brief Clamp raw status values to the supported transaction status enum range. */
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

TransactionController::TransactionController(core::application::AppStateFacade* core,
                                             QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QVariantMap TransactionController::transaction(const QString& id) const
{
    if (!core_) {
        return {};
    }

    const auto& items = core_->state().transactions;
    const auto it = std::find_if(items.begin(), items.end(), [&](const auto& item) {
        return item && QString::fromStdString(item->id) == id;
    });
    return it != items.end() && *it ? ui::payload::entity::toPayload(**it) : QVariantMap{};
}

QVariantList TransactionController::transactions() const
{
    return core_ ? ui::payload::entity::toPayloadList(core_->state().transactions) : QVariantList{};
}

QString TransactionController::addTransaction(const QString& name,
                                              const QString& bookingDate,
                                              double amount,
                                              const QString& description,
                                              const QString& statementId,
                                              int status,
                                              const QString& actorId,
                                              bool allocatable,
                                              const QStringList& propertyIds)
{
  return ui::util::guard::invokeValue<QString>(
        core_, observability::origins::controller::transaction::kAdd, {}, [&]() {
            return QString::fromStdString(core_->addTransaction(
                strings::toStdString(name),
                strings::toStdString(bookingDate),
                amount,
                strings::toStdString(description),
                strings::toStdString(statementId),
                toTransactionStatus(status),
                strings::toStdString(actorId),
                allocatable,
                strings::toStdList(propertyIds)));
        });
}

void TransactionController::updateTransaction(const QString& id,
                                              const QString& name,
                                              const QString& bookingDate,
                                              double amount,
                                              const QString& description,
                                              const QString& statementId,
                                              int status,
                                              const QString& actorId,
                                              bool allocatable,
                                              const QStringList& propertyIds)
{
  ui::util::guard::invokeVoid(
        core_, observability::origins::controller::transaction::kUpdate, [&]() {
            core_->updateTransaction(strings::toStdString(id),
                                     strings::toStdString(name),
                                     strings::toStdString(bookingDate),
                                     amount,
                                     strings::toStdString(description),
                                     strings::toStdString(statementId),
                                     toTransactionStatus(status),
                                     strings::toStdString(actorId),
                                     allocatable,
                                     strings::toStdList(propertyIds));
        });
}

void TransactionController::deleteTransaction(const QString& id)
{
  ui::util::guard::invokeVoid(
        core_, observability::origins::controller::transaction::kDelete,
        [&]() { core_->deleteTransaction(strings::toStdString(id)); });
}

} // namespace ui
