/**
 * @file ui/include/ui/payload/EntityPayloadMapper.h
 * @brief Maps core domain entities to QML-friendly QVariant payloads.
 */

#pragma once

#include <QVariantList>
#include <QVariantMap>

#include <memory>
#include <vector>

#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"

#include "ui/payload/PayloadKeys.h"
#include "ui/payload/PayloadMapper.h"

namespace ui::payload::entity {

/** @brief Convert an actor into a QVariantMap payload. */
inline QVariantMap toPayload(const core::domain::Actor& actor)
{
    QVariantMap payload;
    payload[ui::payload::keys::common::kId] = QString::fromStdString(actor.id);
    payload[ui::payload::keys::common::kName] = QString::fromStdString(actor.name);
    payload[ui::payload::keys::common::kType] = QString::fromStdString(actor.type);
    payload[ui::payload::keys::common::kDescription] = QString::fromStdString(actor.description);
    payload[ui::payload::keys::actor::kAliases] = ui::payload::mapper::toVariantStringList(actor.aliases);
    return payload;
}

/** @brief Convert a property into a QVariantMap payload. */
inline QVariantMap toPayload(const core::domain::Property& property)
{
    QVariantMap payload;
    payload[ui::payload::keys::common::kId] = QString::fromStdString(property.id);
    payload[ui::payload::keys::common::kName] = QString::fromStdString(property.name);
    payload[ui::payload::keys::property::kAddress] = QString::fromStdString(property.address);
    payload[ui::payload::keys::common::kDescription] = QString::fromStdString(property.description);
    payload[ui::payload::keys::property::kConsumption] = property.consumption;
    payload[ui::payload::keys::property::kConsumptionUnit] = QString::fromStdString(property.consumptionUnit);
    payload[ui::payload::keys::property::kAliases] = ui::payload::mapper::toVariantStringList(property.aliases);
    return payload;
}

/** @brief Convert a contract into a QVariantMap payload. */
inline QVariantMap toPayload(const core::domain::Contract& contract)
{
    QVariantMap payload;
    payload[ui::payload::keys::common::kId] = QString::fromStdString(contract.id);
    payload[ui::payload::keys::common::kName] = QString::fromStdString(contract.name);
    payload[ui::payload::keys::common::kType] = QString::fromStdString(contract.type);
    payload[ui::payload::keys::common::kDescription] = QString::fromStdString(contract.description);
    payload[ui::payload::keys::contract::kStartDate] = QString::fromStdString(contract.startDate);
    payload[ui::payload::keys::contract::kEndDate] = QString::fromStdString(contract.endDate);
    payload[ui::payload::keys::contract::kBasePrice] = contract.basePrice;
    payload[ui::payload::keys::contract::kConsumptionPrice] = contract.consumptionPrice;
    payload[ui::payload::keys::contract::kMonthlyAdvance] = contract.monthlyAdvance;
    payload[ui::payload::keys::contract::kActorIds] = ui::payload::mapper::toVariantStringList(contract.actorIds);
    payload[ui::payload::keys::contract::kPropertyIds] = ui::payload::mapper::toVariantStringList(contract.propertyIds);
    payload[ui::payload::keys::contract::kAliases] = ui::payload::mapper::toVariantStringList(contract.aliases);
    return payload;
}

/** @brief Convert a statement into a QVariantMap payload. */
inline QVariantMap toPayload(const core::domain::Statement& statement)
{
    QVariantMap payload;
    payload[ui::payload::keys::common::kId] = QString::fromStdString(statement.id);
    payload[ui::payload::keys::common::kName] = QString::fromStdString(statement.name);
    return payload;
}

/** @brief Convert a transaction into a QVariantMap payload. */
inline QVariantMap toPayload(const core::domain::Transaction& transaction)
{
    QVariantMap payload;
    payload[ui::payload::keys::common::kId] = QString::fromStdString(transaction.id);
    payload[ui::payload::keys::common::kName] = QString::fromStdString(transaction.name);
    payload[ui::payload::keys::transaction::kBookingDate] = QString::fromStdString(transaction.bookingDate);
    payload[ui::payload::keys::transaction::kValuta] = QString::fromStdString(transaction.valuta);
    payload[ui::payload::keys::common::kAmount] = transaction.amount;
    payload[ui::payload::keys::common::kStatus] = static_cast<int>(transaction.status);
    payload[ui::payload::keys::transaction::kContractId] = QString::fromStdString(transaction.contractId);
    payload[ui::payload::keys::transaction::kActorId] = QString::fromStdString(transaction.actorId);
    payload[ui::payload::keys::statement::kStatementId] = QString::fromStdString(transaction.statementId);
    payload[ui::payload::keys::common::kDescription] = QString::fromStdString(transaction.description);
    payload[ui::payload::keys::transaction::kAllocatable] = transaction.allocatable;
    payload[ui::payload::keys::transaction::kPropertyIds] = ui::payload::mapper::toVariantStringList(transaction.propertyIds);
    return payload;
}

/** @brief Convert an annual aggregate into a QVariantMap payload. */
inline QVariantMap toPayload(const core::domain::Annual& annual)
{
    QVariantMap payload;
    payload[ui::payload::keys::common::kId] = QString::fromStdString(annual.id);
    payload[ui::payload::keys::annual::kYear] = annual.year;
    payload[ui::payload::keys::annual::kVerificationState] = static_cast<int>(annual.verificationState);
    payload[ui::payload::keys::annual::kTransactionIds] = ui::payload::mapper::toVariantStringList(annual.transactionIds);
    payload[ui::payload::keys::annual::kAssignedAnalysisIds] = ui::payload::mapper::toVariantStringList(annual.assignedAnalysisIds);
    return payload;
}

/** @brief Convert an analysis definition into a QVariantMap payload. */
inline QVariantMap toPayload(const core::domain::Analysis& analysis)
{
    QVariantMap payload;
    payload[ui::payload::keys::common::kId] = QString::fromStdString(analysis.id);
    payload[ui::payload::keys::common::kName] = QString::fromStdString(analysis.name);
    payload[ui::payload::keys::common::kType] = QString::fromStdString(analysis.type);
    payload[ui::payload::keys::analysis::kConfig] = QString::fromStdString(analysis.configJson);
    payload[ui::payload::keys::analysis::kFilter] = QString::fromStdString(analysis.filterSpec);
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
