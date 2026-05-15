/**
 * @file ui/include/ui/adapters/core/EntityPayloadMapper.h
 * @brief Maps core domain entities to QML-friendly QVariant payloads.
 */

#pragma once

#include <QVariantList>
#include <QVariantMap>

#include <memory>
#include <vector>

#include "core/domain/values/Alias.h"
#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"

#include "ui/shared/payload/PayloadKeys.h"
#include "ui/shared/payload/PayloadMapper.h"

namespace ui::payload::entity {

inline QVariantMap toPayload(const core::domain::Alias& alias)
{
    QVariantMap payload;
    payload[QStringLiteral("value")] = QString::fromStdString(alias.value());
    payload[QStringLiteral("kind")] = QString::fromStdString(alias.kind());
    payload[QStringLiteral("source")] = QString::fromStdString(alias.source());
    payload[QStringLiteral("createdAt")] = QString::fromStdString(alias.createdAt());
    payload[QStringLiteral("updatedAt")] = QString::fromStdString(alias.updatedAt());
    return payload;
}

/** @brief Convert an actor into a QVariantMap payload. */
inline QVariantMap toPayload(const core::domain::Actor& actor)
{
    QVariantMap payload;
    payload[ui::payload::keys::common::kId] = QString::fromStdString(actor.id());
    payload[ui::payload::keys::common::kName] = QString::fromStdString(actor.name());
    payload[QStringLiteral("createdAt")] = QString::fromStdString(actor.createdAt());
    payload[QStringLiteral("updatedAt")] = QString::fromStdString(actor.updatedAt());
    QVariantList aliases;
    aliases.reserve(static_cast<int>(actor.aliases().size()));
    for (const auto& alias : actor.aliases()) {
        aliases.push_back(toPayload(alias));
    }
    payload[ui::payload::keys::actor::kAliases] = aliases;
    return payload;
}

/** @brief Convert a property into a QVariantMap payload. */
inline QVariantMap toPayload(const core::domain::Property& property)
{
    QVariantMap payload;
    payload[ui::payload::keys::common::kId] = QString::fromStdString(property.id());
    payload[ui::payload::keys::common::kName] = QString::fromStdString(property.name());
    payload[QStringLiteral("createdAt")] = QString::fromStdString(property.createdAt());
    payload[QStringLiteral("updatedAt")] = QString::fromStdString(property.updatedAt());
    QVariantList aliases;
    aliases.reserve(static_cast<int>(property.aliases().size()));
    for (const auto& alias : property.aliases()) {
        aliases.push_back(toPayload(alias));
    }
    payload[ui::payload::keys::property::kAliases] = aliases;
    return payload;
}

/** @brief Convert a contract into a QVariantMap payload. */
inline QVariantMap toPayload(const core::domain::Contract& contract)
{
    QVariantMap payload;
    payload[ui::payload::keys::common::kId] = QString::fromStdString(contract.id());
    payload[ui::payload::keys::common::kName] = QString::fromStdString(contract.name());
    payload[ui::payload::keys::common::kType] = QString::fromStdString(contract.type());
    payload[QStringLiteral("createdAt")] = QString::fromStdString(contract.createdAt());
    payload[QStringLiteral("updatedAt")] = QString::fromStdString(contract.updatedAt());
    payload[ui::payload::keys::contract::kActorIds] = ui::payload::mapper::toVariantStringList(contract.actorIds());
    payload[ui::payload::keys::contract::kPropertyIds] = ui::payload::mapper::toVariantStringList(contract.propertyIds());
    QVariantList aliases;
    aliases.reserve(static_cast<int>(contract.aliases().size()));
    for (const auto& alias : contract.aliases()) {
        aliases.push_back(toPayload(alias));
    }
    payload[ui::payload::keys::contract::kAliases] = aliases;
    return payload;
}

/** @brief Convert a statement into a QVariantMap payload. */
inline QVariantMap toPayload(const core::domain::Statement& statement)
{
    QVariantMap payload;
    payload[ui::payload::keys::common::kId] = QString::fromStdString(statement.id());
    payload[ui::payload::keys::common::kName] = QString::fromStdString(statement.name());
    return payload;
}

/** @brief Convert a transaction into a QVariantMap payload. */
inline QVariantMap toPayload(const core::domain::Transaction& transaction)
{
    QVariantMap payload;
    payload[ui::payload::keys::common::kId] = QString::fromStdString(transaction.id());
    payload[ui::payload::keys::common::kName] = QString::fromStdString(transaction.name());
    payload[ui::payload::keys::transaction::kBookingDate] = QString::fromStdString(transaction.bookingDate());
    payload[ui::payload::keys::transaction::kValuta] = QString::fromStdString(transaction.valuta());
    payload[ui::payload::keys::common::kAmount] = transaction.amount();
    payload[ui::payload::keys::common::kStatus] = static_cast<int>(transaction.status());
    payload[ui::payload::keys::transaction::kContractId] = QString::fromStdString(transaction.contractId());
    payload[ui::payload::keys::transaction::kActorId] = QString::fromStdString(transaction.actorId());
    payload[ui::payload::keys::statement::kStatementId] = QString::fromStdString(transaction.statementId());
    payload[ui::payload::keys::transaction::kAllocatable] = transaction.isAllocatable();
    payload[ui::payload::keys::transaction::kPropertyIds] = ui::payload::mapper::toVariantStringList(transaction.propertyIds());
    return payload;
}

/** @brief Convert an annual aggregate into a QVariantMap payload. */
inline QVariantMap toPayload(const core::domain::Annual& annual)
{
    QVariantMap payload;
    payload[ui::payload::keys::common::kId] = QString::fromStdString(annual.id());
    payload[ui::payload::keys::annual::kName] = QString::fromStdString(annual.name());
    payload[ui::payload::keys::annual::kYear] = annual.year();
    payload[ui::payload::keys::annual::kAnalysisIds] = ui::payload::mapper::toVariantStringList(annual.analysisIds());
    return payload;
}

/** @brief Convert an analysis definition into a QVariantMap payload. */
inline QVariantMap toPayload(const core::domain::Analysis& analysis)
{
    QVariantMap payload;
    payload[ui::payload::keys::common::kId] = QString::fromStdString(analysis.id());
    payload[ui::payload::keys::common::kName] = QString::fromStdString(analysis.name());
    payload[ui::payload::keys::common::kType] = QString::fromStdString(analysis.type());
    payload[ui::payload::keys::analysis::kConfig] = QString::fromStdString(analysis.configJson());
    payload[ui::payload::keys::analysis::kFilter] = QString::fromStdString(analysis.filterSpec());
    payload[ui::payload::keys::analysis::kExportFormat] = QString::fromStdString(analysis.exportFormat());
    payload[ui::payload::keys::analysis::kIncludeCalcAdjustments] = analysis.includeCalculationAdjustments();
    payload[ui::payload::keys::analysis::kExportState] = QString::fromStdString(analysis.exportStateJson());
    payload[ui::payload::keys::analysis::kSnapshotTransactions] = QString::fromStdString(analysis.snapshotTransactionsJson());
    payload[ui::payload::keys::analysis::kCreatedAt] = QString::fromStdString(analysis.createdAt());
    payload[ui::payload::keys::analysis::kUpdatedAt] = QString::fromStdString(analysis.updatedAt());
    return payload;
}

/**
 * @brief Convert a collection of shared entities into a QVariantList payload.
 * @tparam T Entity type.
 * @param items Shared entity collection.
 * @return QVariantList of QVariantMap payloads.
 */
template <typename T>
QVariantList toPayloadList(const std::vector<std::shared_ptr<T>>& items)
{
    QVariantList payload;
    payload.reserve(static_cast<int>(items.size()));
    for (const auto& item : items) {
        if (item) {
            payload.push_back(toPayload(*item));
        }
    }
    return payload;
}

} // namespace ui::payload::entity

