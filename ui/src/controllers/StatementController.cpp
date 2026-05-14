/**
 * @file ui/src/controllers/StatementController.cpp
 * @brief Implements the UI controller that forwards statement mutations to the application facade.
 */

#include "ui/controllers/StatementController.h"

#include <algorithm>

#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/observability/Origins.h"
#include "ui/payload/PayloadMapper.h"
#include "ui/util/CoreFacadeGuard.h"
#include "ui/util/StringConversions.h"

namespace ui {

StatementController::StatementController(core::ports::workspace::IWorkspaceWriter* core,
                                         QObject* parent)
    : QObject(parent)
    , core_(core)
    , reader_(dynamic_cast<core::ports::workspace::IWorkspaceReader*>(core))
{
}

QVariantMap StatementController::statement(const QString& id) const
{
    if (!reader_) {
        return {};
    }

    const auto snapshot = reader_->workspaceSnapshot();
    const auto& statements = snapshot.statements;
    const auto statementIt = std::find_if(statements.begin(), statements.end(), [&](const auto& item) {
        return QString::fromStdString(item.id) == id;
    });
    if (statementIt != statements.end()) {
        QVariantMap payload;
        payload[QStringLiteral("id")] = QString::fromStdString(statementIt->id);
        payload[QStringLiteral("name")] = QString::fromStdString(statementIt->name);
        payload[QStringLiteral("transactionIds")] = ui::payload::mapper::toVariantStringList(statementIt->transactionIds);
        payload[QStringLiteral("createdAt")] = QString::fromStdString(statementIt->createdAt);
        payload[QStringLiteral("updatedAt")] = QString::fromStdString(statementIt->updatedAt);
        return payload;
    }

    const auto& transactions = snapshot.transactions;
    const auto transactionIt = std::find_if(transactions.begin(), transactions.end(), [&](const auto& item) {
        return QString::fromStdString(item.id) == id;
    });
    if (transactionIt == transactions.end()) {
        return {};
    }

    QVariantMap payload;
    payload[QStringLiteral("id")] = QString::fromStdString(transactionIt->id);
    payload[QStringLiteral("name")] = QString::fromStdString(transactionIt->name);
    payload[QStringLiteral("bookingDate")] = QString::fromStdString(transactionIt->bookingDate);
    payload[QStringLiteral("valuta")] = QString::fromStdString(transactionIt->valuta);
    payload[QStringLiteral("amount")] = transactionIt->amount;
    payload[QStringLiteral("status")] = static_cast<int>(transactionIt->status);
    payload[QStringLiteral("contractId")] = QString::fromStdString(transactionIt->contractId);
    payload[QStringLiteral("actorId")] = QString::fromStdString(transactionIt->actorId);
    payload[QStringLiteral("statementId")] = QString::fromStdString(transactionIt->statementId);
    payload[QStringLiteral("allocatable")] = transactionIt->allocatable;
    payload[QStringLiteral("propertyIds")] = ui::payload::mapper::toVariantStringList(transactionIt->propertyIds);
    payload[QStringLiteral("createdAt")] = QString::fromStdString(transactionIt->createdAt);
    payload[QStringLiteral("updatedAt")] = QString::fromStdString(transactionIt->updatedAt);
    return payload;
}

QVariantList StatementController::statements() const
{
    if (!reader_) {
        return {};
    }

    QVariantList out;
    const auto items = reader_->workspaceSnapshot().statements;
    out.reserve(static_cast<int>(items.size()));
    for (const auto& item : items) {
        QVariantMap payload;
        payload[QStringLiteral("id")] = QString::fromStdString(item.id);
        payload[QStringLiteral("name")] = QString::fromStdString(item.name);
        payload[QStringLiteral("transactionIds")] = ui::payload::mapper::toVariantStringList(item.transactionIds);
        payload[QStringLiteral("createdAt")] = QString::fromStdString(item.createdAt);
        payload[QStringLiteral("updatedAt")] = QString::fromStdString(item.updatedAt);
        out.push_back(std::move(payload));
    }
    return out;
}

QString StatementController::addStatement(const QString& name)
{
    return ui::util::guard::invokeValue<QString>(
        core_, observability::origins::controller::statement::kAdd, {}, [&]() {
            core::ports::workspace::StatementCommand command;
            command.name = strings::toStdString(name);
            return QString::fromStdString(core_->addStatement(command));
        });
}

void StatementController::updateStatement(const QString& id,
                                          const QString& name)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::controller::statement::kUpdate, [&]() {
            core::ports::workspace::StatementCommand command;
            command.id = strings::toStdString(id);
            command.name = strings::toStdString(name);
            core_->updateStatement(command);
        });
}


void StatementController::deleteStatement(const QString& id)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::controller::statement::kDelete,
        [&]() { core_->deleteStatement(strings::toStdString(id)); });
}

} // namespace ui
