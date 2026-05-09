/**
 * @file core/src/application/CatalogDraftAppliers.h
 * @brief Declares private draft-application helpers used by `WorkspaceCatalogService`.
 */

#pragma once

#include "core/application/workspace/WorkspaceCatalogService.h"
#include "core/domain/values/Alias.h"
#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"
#include "core/utils/Time.h"
#include "../../utils/Util.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace core::application::detail {

inline void normalizeAlias(core::domain::Alias& alias)
{
    const std::string now = core::utils::currentTimestampUtc();
    if (alias.value.empty()) alias.value = alias.source;
    if (alias.source.empty()) alias.source = alias.value;
    if (alias.createdAt.empty()) alias.createdAt = now;
    if (alias.updatedAt.empty()) alias.updatedAt = now;
}

inline void normalizeAliases(std::vector<core::domain::Alias>& aliases)
{
    for (auto& alias : aliases) {
        normalizeAlias(alias);
    }
}

inline void resetTransientTransactionFields(Transaction& tx)
{
    tx.valuta.clear();
}

inline void applyActorDraft(Actor& actor, const ActorInput& input)
{
    actor.name = input.name;
    actor.aliases = input.aliases;
    normalizeAliases(actor.aliases);
}

inline void applyPropertyDraft(Property& property, const PropertyInput& input)
{
    property.name = input.name;
    property.aliases = input.aliases;
    normalizeAliases(property.aliases);
}

inline void applyContractDraft(Contract& contract, const ContractInput& input)
{
    contract.name = input.name;
    contract.type = input.type;
    contract.actorIds = input.actorIds;
    contract.propertyIds = input.propertyIds;
    contract.aliases = input.aliases;
    normalizeAliases(contract.aliases);
}

inline void applyStatementName(Statement& statement, const std::string& name)
{
    statement.name = name;
}

inline void applyAnalysisDraft(Analysis& analysis, const AnalysisInput& input)
{
    analysis.name = input.name;
    analysis.type = input.type;
    analysis.configJson = input.configJson;
    analysis.filterSpec = input.filterSpec;
    analysis.exportFormat = input.exportFormat;
    analysis.includeCalcAdjustments = input.includeCalcAdjustments;
    analysis.exportStateJson = input.exportStateJson;
    analysis.snapshotTransactionsJson = input.snapshotTransactionsJson;
}

inline void applyAnnualDraft(Annual& annual,
                             const std::string& name,
                             int year,
                             const std::vector<std::string>& analysisIds)
{
    annual.name = name;
    annual.year = year;
    annual.analysisIds = analysisIds;
}

inline void applyTransactionDraft(Transaction& tx, const TransactionInput& input)
{
    tx.name = input.name;
    tx.bookingDate = input.bookingDate;
    tx.amount = input.amount;
    tx.statementId = input.statementId;
    tx.status = input.status;
    tx.actorId = input.actorId;
    tx.allocatable = input.allocatable;
    tx.propertyIds = input.propertyIds;
}

inline bool isBlank(const std::string& value)
{
    return std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c) != 0; });
}

} // namespace core::application::detail
