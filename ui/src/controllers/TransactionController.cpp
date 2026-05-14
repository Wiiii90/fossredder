/**
 * @file ui/src/controllers/TransactionController.cpp
 * @brief Implements the UI controller that forwards transaction mutations to the application facade.
 */

#include "ui/controllers/TransactionController.h"

#include <algorithm>

#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/observability/Origins.h"
#include "ui/payload/PayloadMapper.h"
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

TransactionController::TransactionController(core::ports::workspace::IWorkspaceWriter* core,
                                             QObject* parent)
    : QObject(parent)
    , core_(core)
    , reader_(dynamic_cast<core::ports::workspace::IWorkspaceReader*>(core))
{
}

QVariantMap TransactionController::transaction(const QString& id) const
{
    if (!reader_) {
        return {};
    }

    const auto items = reader_->workspaceSnapshot().transactions;
    const auto it = std::find_if(items.begin(), items.end(), [&](const auto& item) {
        return QString::fromStdString(item.id) == id;
    });
    if (it == items.end()) {
        return {};
    }

    QVariantMap payload;
    payload[QStringLiteral("id")] = QString::fromStdString(it->id);
    payload[QStringLiteral("name")] = QString::fromStdString(it->name);
    payload[QStringLiteral("bookingDate")] = QString::fromStdString(it->bookingDate);
    payload[QStringLiteral("valuta")] = QString::fromStdString(it->valuta);
    payload[QStringLiteral("amount")] = it->amount;
    payload[QStringLiteral("status")] = static_cast<int>(it->status);
    payload[QStringLiteral("contractId")] = QString::fromStdString(it->contractId);
    payload[QStringLiteral("actorId")] = QString::fromStdString(it->actorId);
    payload[QStringLiteral("statementId")] = QString::fromStdString(it->statementId);
    payload[QStringLiteral("allocatable")] = it->allocatable;
    payload[QStringLiteral("propertyIds")] = ui::payload::mapper::toVariantStringList(it->propertyIds);
    payload[QStringLiteral("createdAt")] = QString::fromStdString(it->createdAt);
    payload[QStringLiteral("updatedAt")] = QString::fromStdString(it->updatedAt);
    return payload;
}

QVariantList TransactionController::transactions() const
{
    if (!reader_) {
        return {};
    }

    QVariantList out;
    const auto items = reader_->workspaceSnapshot().transactions;
    out.reserve(static_cast<int>(items.size()));
    for (const auto& item : items) {
        QVariantMap payload;
        payload[QStringLiteral("id")] = QString::fromStdString(item.id);
        payload[QStringLiteral("name")] = QString::fromStdString(item.name);
        payload[QStringLiteral("bookingDate")] = QString::fromStdString(item.bookingDate);
        payload[QStringLiteral("valuta")] = QString::fromStdString(item.valuta);
        payload[QStringLiteral("amount")] = item.amount;
        payload[QStringLiteral("status")] = static_cast<int>(item.status);
        payload[QStringLiteral("contractId")] = QString::fromStdString(item.contractId);
        payload[QStringLiteral("actorId")] = QString::fromStdString(item.actorId);
        payload[QStringLiteral("statementId")] = QString::fromStdString(item.statementId);
        payload[QStringLiteral("allocatable")] = item.allocatable;
        payload[QStringLiteral("propertyIds")] = ui::payload::mapper::toVariantStringList(item.propertyIds);
        payload[QStringLiteral("createdAt")] = QString::fromStdString(item.createdAt);
        payload[QStringLiteral("updatedAt")] = QString::fromStdString(item.updatedAt);
        out.push_back(std::move(payload));
    }
    return out;
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

                core::ports::workspace::TransactionCommand command;
                command.name = strings::toStdString(draft.value(QStringLiteral("name")).toString());
                command.bookingDate = strings::toStdString(draft.value(QStringLiteral("bookingDate")).toString());
                command.amount = draft.value(QStringLiteral("amount")).toDouble();
                command.statementId = strings::toStdString(statementId);
                command.status = toTransactionStatus(draft.value(QStringLiteral("status")).toInt());
                command.actorId = strings::toStdString(draft.value(QStringLiteral("actorId")).toString());
                command.allocatable = draft.value(QStringLiteral("allocatable")).toBool();
                command.propertyIds = strings::toStdList(draft.value(QStringLiteral("propertyIds")).toStringList());
                const std::string createdId = core_->addTransaction(command);

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
            core::ports::workspace::TransactionCommand command;
            command.name = strings::toStdString(name);
            command.bookingDate = strings::toStdString(bookingDate);
            command.amount = amount;
            command.statementId = strings::toStdString(statementId);
            command.status = toTransactionStatus(status);
            command.actorId = strings::toStdString(actorId);
            command.allocatable = allocatable;
            command.propertyIds = strings::toStdList(propertyIds);
            return QString::fromStdString(core_->addTransaction(command));
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
            core::ports::workspace::TransactionCommand command;
            command.id = strings::toStdString(id);
            command.name = strings::toStdString(name);
            command.bookingDate = strings::toStdString(bookingDate);
            command.amount = amount;
            command.statementId = strings::toStdString(statementId);
            command.status = toTransactionStatus(status);
            command.actorId = strings::toStdString(actorId);
            command.allocatable = allocatable;
            command.propertyIds = strings::toStdList(propertyIds);
            core_->updateTransaction(command);
        });
}

void TransactionController::deleteTransaction(const QString& id)
{
  ui::util::guard::invokeVoid(
        core_, observability::origins::controller::transaction::kDelete,
        [&]() { core_->deleteTransaction(strings::toStdString(id)); });
}

} // namespace ui
