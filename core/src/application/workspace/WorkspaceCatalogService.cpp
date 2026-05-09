/**
 * @file core/src/application/workspace/WorkspaceCatalogService.cpp
 * @brief Implements application services for mutating workspace catalog entities in the app state.
 */

#include "core/application/workspace/WorkspaceCatalogService.h"

#include "CatalogDraftAppliers.h"
#include "CatalogMutationHelpers.h"

#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"
#include "../../utils/Util.h"

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

std::string WorkspaceCatalogService::addActor(WorkspaceState& state, const ActorInput& input) const
{
    return appendEntity(state.actors, [&](Actor& actor) { applyActorDraft(actor, input); });
}

bool WorkspaceCatalogService::updateActor(WorkspaceState& state, const std::string& id, const ActorInput& input) const
{
    return updateEntity(state.actors, id, [&](Actor& actor) { applyActorDraft(actor, input); });
}

bool WorkspaceCatalogService::deleteActor(WorkspaceState& state, const std::string& id) const
{
    return eraseEntity(state.actors, id);
}

std::string WorkspaceCatalogService::addProperty(WorkspaceState& state, const PropertyInput& input) const
{
    return appendEntity(state.properties, [&](Property& property) { applyPropertyDraft(property, input); });
}

bool WorkspaceCatalogService::updateProperty(WorkspaceState& state, const std::string& id, const PropertyInput& input) const
{
    return updateEntity(state.properties, id, [&](Property& property) { applyPropertyDraft(property, input); });
}

bool WorkspaceCatalogService::deleteProperty(WorkspaceState& state, const std::string& id) const
{
    return eraseEntity(state.properties, id);
}

std::string WorkspaceCatalogService::addContract(WorkspaceState& state, const ContractInput& input) const
{
    return appendEntity(state.contracts, [&](Contract& contract) { applyContractDraft(contract, input); });
}

bool WorkspaceCatalogService::updateContract(WorkspaceState& state, const std::string& id, const ContractInput& input) const
{
    return updateEntity(state.contracts, id, [&](Contract& contract) { applyContractDraft(contract, input); });
}

bool WorkspaceCatalogService::deleteContract(WorkspaceState& state, const std::string& id) const
{
    return eraseEntity(state.contracts, id);
}

std::string WorkspaceCatalogService::addStatement(WorkspaceState& state, const std::string& name) const
{
    return appendEntity(state.statements, [&](Statement& statement) { applyStatementName(statement, name); });
}

bool WorkspaceCatalogService::updateStatement(WorkspaceState& state, const std::string& id, const std::string& name) const
{
    return updateEntity(state.statements, id, [&](Statement& statement) { applyStatementName(statement, name); });
}

bool WorkspaceCatalogService::deleteStatement(WorkspaceState& state, const std::string& id) const
{
    if (id.empty()) return false;

    const auto originalTransactionCount = state.transactions.size();
    state.transactions.erase(std::remove_if(state.transactions.begin(), state.transactions.end(), [&](const auto& tx) {
        return tx && tx->statementId == id;
    }), state.transactions.end());

    const bool removedStatement = eraseEntity(state.statements, id);
    return removedStatement || state.transactions.size() != originalTransactionCount;
}

std::string WorkspaceCatalogService::addTransaction(WorkspaceState& state, const TransactionInput& input) const
{
    if (isBlank(input.statementId)) return {};

    return appendEntity(state.transactions, [&](Transaction& tx) {
        applyTransactionDraft(tx, input);
        resetTransientTransactionFields(tx);
    });
}

bool WorkspaceCatalogService::updateTransaction(WorkspaceState& state, const std::string& id, const TransactionInput& input) const
{
    return updateEntity(state.transactions, id, [&](Transaction& tx) {
        applyTransactionDraft(tx, input);
    });
}

bool WorkspaceCatalogService::deleteTransaction(WorkspaceState& state, const std::string& id) const
{
    return eraseEntity(state.transactions, id);
}

std::string WorkspaceCatalogService::addAnalysis(WorkspaceState& state, const AnalysisInput& input) const
{
    return appendEntity(state.analyses, [&](Analysis& analysis) { applyAnalysisDraft(analysis, input); });
}

bool WorkspaceCatalogService::updateAnalysis(WorkspaceState& state, const std::string& id, const AnalysisInput& input) const
{
    return updateEntity(state.analyses, id, [&](Analysis& analysis) { applyAnalysisDraft(analysis, input); });
}

bool WorkspaceCatalogService::deleteAnalysis(WorkspaceState& state, const std::string& id) const
{
    return eraseEntity(state.analyses, id);
}

std::string WorkspaceCatalogService::addAnnual(WorkspaceState& state,
                                      const std::string& name,
                                      int year,
                                      const std::vector<std::string>& analysisIds) const
{
    return appendEntity(state.annuals,
                        [&](Annual& annual) { applyAnnualDraft(annual, name, year, analysisIds); });
}

bool WorkspaceCatalogService::updateAnnual(WorkspaceState& state,
                                  const std::string& id,
                                  const std::string& name,
                                  int year,
                                  const std::vector<std::string>& analysisIds) const
{
    return updateEntity(state.annuals,
                        id,
                        [&](Annual& annual) { applyAnnualDraft(annual, name, year, analysisIds); });
}

bool WorkspaceCatalogService::deleteAnnual(WorkspaceState& state, const std::string& id) const
{
    return eraseEntity(state.annuals, id);
}

std::vector<std::string> WorkspaceCatalogService::contractTypes(const WorkspaceState& state) const
{
    std::unordered_set<std::string> seen;
    std::vector<std::string> values;
    values.reserve(state.contracts.size());

    for (const auto& contract : state.contracts) {
        if (!contract) continue;
        const std::string type = ::core::utils::trim(contract->type);
        if (type.empty()) continue;
        if (!seen.insert(type).second) continue;
        values.push_back(type);
    }

    std::sort(values.begin(), values.end());
    return values;
}

}
