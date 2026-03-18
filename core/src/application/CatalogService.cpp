/**
 * @file core/src/application/CatalogService.cpp
 * @brief Implements application services for mutating catalog entities in the app state.
 */

#include "core/application/CatalogService.h"

#include "CatalogDraftAppliers.h"
#include "CatalogMutationHelpers.h"

#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "../utils/Util.h"

#include <unordered_set>

namespace core::application {

using detail::appendEntity;
using detail::updateEntity;
using detail::eraseEntity;
using detail::resetTransientTransactionFields;
using detail::applyActorDraft;
using detail::applyPropertyDraft;
using detail::applyContractDraft;
using detail::applyStatementName;
using detail::applyAnalysisDraft;
using detail::applyAnnualDraft;
using detail::applyTransactionDraft;
using detail::isBlank;

std::string CatalogService::addActor(AppState& state, const ActorInput& input) const
{
    return appendEntity(state.actors, [&](Actor& actor) { applyActorDraft(actor, input); });
}

bool CatalogService::updateActor(AppState& state, const std::string& id, const ActorInput& input) const
{
    return updateEntity(state.actors, id, [&](Actor& actor) { applyActorDraft(actor, input); });
}

bool CatalogService::deleteActor(AppState& state, const std::string& id) const
{
    return eraseEntity(state.actors, id);
}

std::string CatalogService::addProperty(AppState& state, const PropertyInput& input) const
{
    return appendEntity(state.properties, [&](Property& property) { applyPropertyDraft(property, input); });
}

bool CatalogService::updateProperty(AppState& state, const std::string& id, const PropertyInput& input) const
{
    return updateEntity(state.properties, id, [&](Property& property) { applyPropertyDraft(property, input); });
}

bool CatalogService::deleteProperty(AppState& state, const std::string& id) const
{
    return eraseEntity(state.properties, id);
}

std::string CatalogService::addContract(AppState& state, const ContractInput& input) const
{
    return appendEntity(state.contracts, [&](Contract& contract) { applyContractDraft(contract, input); });
}

bool CatalogService::updateContract(AppState& state, const std::string& id, const ContractInput& input) const
{
    return updateEntity(state.contracts, id, [&](Contract& contract) { applyContractDraft(contract, input); });
}

bool CatalogService::deleteContract(AppState& state, const std::string& id) const
{
    return eraseEntity(state.contracts, id);
}

std::string CatalogService::addStatement(AppState& state, const std::string& name) const
{
    return appendEntity(state.statements, [&](Statement& statement) { applyStatementName(statement, name); });
}

bool CatalogService::updateStatement(AppState& state, const std::string& id, const std::string& name) const
{
    return updateEntity(state.statements, id, [&](Statement& statement) { applyStatementName(statement, name); });
}

bool CatalogService::deleteStatement(AppState& state, const std::string& id) const
{
    if (id.empty()) return false;

    const auto originalTransactionCount = state.transactions.size();
    state.transactions.erase(std::remove_if(state.transactions.begin(), state.transactions.end(), [&](const auto& tx) {
        return tx && tx->statementId == id;
    }), state.transactions.end());

    const bool removedStatement = eraseEntity(state.statements, id);
    return removedStatement || state.transactions.size() != originalTransactionCount;
}

std::string CatalogService::addTransaction(AppState& state, const TransactionInput& input) const
{
    if (isBlank(input.statementId)) return {};

    return appendEntity(state.transactions, [&](Transaction& tx) {
        applyTransactionDraft(tx, input);
        resetTransientTransactionFields(tx);
    });
}

bool CatalogService::updateTransaction(AppState& state, const std::string& id, const TransactionInput& input) const
{
    return updateEntity(state.transactions, id, [&](Transaction& tx) {
        applyTransactionDraft(tx, input);
    });
}

bool CatalogService::deleteTransaction(AppState& state, const std::string& id) const
{
    return eraseEntity(state.transactions, id);
}

std::string CatalogService::addAnalysis(AppState& state, const AnalysisInput& input) const
{
    return appendEntity(state.analyses, [&](Analysis& analysis) { applyAnalysisDraft(analysis, input); });
}

bool CatalogService::updateAnalysis(AppState& state, const std::string& id, const AnalysisInput& input) const
{
    return updateEntity(state.analyses, id, [&](Analysis& analysis) { applyAnalysisDraft(analysis, input); });
}

bool CatalogService::deleteAnalysis(AppState& state, const std::string& id) const
{
    return eraseEntity(state.analyses, id);
}

std::string CatalogService::addAnnual(AppState& state, int year) const
{
    return appendEntity(state.annuals, [&](Annual& annual) { applyAnnualDraft(annual, year); });
}

bool CatalogService::updateAnnual(AppState& state, const std::string& id, int year) const
{
    return updateEntity(state.annuals, id, [&](Annual& annual) { applyAnnualDraft(annual, year); });
}

bool CatalogService::deleteAnnual(AppState& state, const std::string& id) const
{
    return eraseEntity(state.annuals, id);
}

std::vector<std::string> CatalogService::contractTypes(const AppState& state) const
{
    std::unordered_set<std::string> seen;
    std::vector<std::string> values;
    values.reserve(state.contracts.size());

    for (const auto& contract : state.contracts) {
        if (!contract) continue;
        const std::string type = ::utils::trim(contract->type);
        if (type.empty()) continue;
        if (!seen.insert(type).second) continue;
        values.push_back(type);
    }

    std::sort(values.begin(), values.end());
    return values;
}

}
