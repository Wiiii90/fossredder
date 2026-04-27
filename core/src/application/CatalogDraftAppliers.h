/**
 * @file core/src/application/CatalogDraftAppliers.h
 * @brief Declares private draft-application helpers used by `CatalogService`.
 */

#pragma once

#include "core/application/CatalogService.h"
#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "../utils/Util.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace core::application::detail {

inline void resetTransientTransactionFields(Transaction& tx)
{
    tx.valuta.clear();
}

inline void applyActorDraft(Actor& actor, const ActorInput& input)
{
    actor.name = input.name;
    actor.type = input.type;
    actor.description = input.description;
    actor.aliases = input.aliases;
}

inline void applyPropertyDraft(Property& property, const PropertyInput& input)
{
    property.name = input.name;
    property.address = input.address;
    property.description = input.description;
    property.aliases = input.aliases;
}

inline void applyContractDraft(Contract& contract, const ContractInput& input)
{
    contract.name = input.name;
    contract.type = input.type;
    contract.description = input.description;
    contract.actorIds = input.actorIds;
    contract.propertyIds = input.propertyIds;
    contract.aliases = input.aliases;
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

inline void applyAnnualDraft(Annual& annual, int year)
{
    annual.year = year;
}

inline void applyTransactionDraft(Transaction& tx, const TransactionInput& input)
{
    tx.name = input.name;
    tx.bookingDate = input.bookingDate;
    tx.amount = input.amount;
    tx.description = input.description;
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
