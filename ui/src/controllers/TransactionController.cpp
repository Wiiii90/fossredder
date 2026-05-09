/**
 * @file ui/src/controllers/TransactionController.cpp
 * @brief Implements the UI controller that forwards transaction mutations to the application facade.
 */

#include "ui/controllers/TransactionController.h"

#include <algorithm>

#include "core/application/workspace/WorkspaceFacade.h"
#include "ui/observability/Origins.h"
#include "ui/payload/EntityPayloadMapper.h"
#include "ui/state/StateFacadeProjection.h"
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

TransactionController::TransactionController(core::application::WorkspaceFacade* core,
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

QVariantList TransactionController::addTransactions(const QString& statementId,
                                                    const QVariantList& transactionDrafts)
{
    return ui::util::guard::invokeValue<QVariantList>(
        core_, observability::origins::controller::transaction::kAdd, {}, [&]() {
            QVariantList createdIds;
            if (statementId.isEmpty()) {
                return createdIds;
            }

            const QVariantList drafts = ui::normalizeTransactionDrafts(transactionDrafts);
            for (const auto& draftValue : drafts) {
                const QVariantMap draft = ui::normalizeTransactionDraft(draftValue.toMap());
                if (!ui::transactionDraftHasContent(draft)) {
                    continue;
                }

                const std::string createdId = core_->addTransaction(
                    strings::toStdString(draft.value(QStringLiteral("name")).toString()),
                    strings::toStdString(draft.value(QStringLiteral("bookingDate")).toString()),
                    draft.value(QStringLiteral("amount")).toDouble(),
                    strings::toStdString(statementId),
                    toTransactionStatus(draft.value(QStringLiteral("status")).toInt()),
                    strings::toStdString(draft.value(QStringLiteral("actorId")).toString()),
                    draft.value(QStringLiteral("allocatable")).toBool(),
                    strings::toStdList(draft.value(QStringLiteral("propertyIds")).toStringList()));

                if (!createdId.empty()) {
                    createdIds.push_back(QString::fromStdString(createdId));
                }
            }
            return createdIds;
        });
}

QString TransactionController::addTransaction(const QString& name,
                                              const QString& bookingDate,
                                              double amount,
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
