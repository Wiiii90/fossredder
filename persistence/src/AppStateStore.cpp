/**
 * @file persistence/src/AppStateStore.cpp
 * @brief Implements SQLite-backed persistence for the aggregate application state.
 */

#include "persistence/AppStateStore.h"

#include "core/application/AppStateManager.h"
#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "core/repositories/IActorRepository.h"
#include "core/repositories/IAnalysisRepository.h"
#include "core/repositories/IAnnualRepository.h"
#include "core/repositories/IContractRepository.h"
#include "core/repositories/IPropertyRepository.h"
#include "core/repositories/IStatementRepository.h"
#include "core/repositories/ITransactionRepository.h"
#include "persistence/SqliteDb.h"
#include "persistence/SqliteTransaction.h"
#include "persistence/Factory.h"

#include <unordered_set>
#include <unordered_map>

namespace {

/** @brief Collects stable ids from a repository result collection. */
template <typename Collection>
std::unordered_set<std::string> collectIds(const Collection& items)
{
    std::unordered_set<std::string> ids;
    ids.reserve(items.size());
    for (const auto& item : items) {
        if (item && !item->id.empty()) ids.insert(item->id);
    }
    return ids;
}

/** @brief Loads a repository collection and extracts its stable ids. */
template <typename RepoPtr, typename LoadFn>
std::unordered_set<std::string> collectRepositoryIds(const RepoPtr& repo, LoadFn&& load)
{
    if (!repo) return {};
    return collectIds(load(*repo));
}

/** @brief Deletes rows that no longer exist in the new application state snapshot. */
template <typename RepoPtr, typename DeleteFn>
void deleteStaleIds(const RepoPtr& repo,
                    const std::unordered_set<std::string>& existingIds,
                    const std::unordered_set<std::string>& currentIds,
                    DeleteFn&& remove,
                    std::vector<std::string>* deletedIds = nullptr)
{
    if (!repo) return;
    for (const auto& id : existingIds) {
        if (currentIds.contains(id)) continue;
        remove(*repo, id);
        if (deletedIds) deletedIds->push_back(id);
    }
}

}

AppStateStore::AppStateStore(std::shared_ptr<SqliteDb> db)
    : db_(std::move(db)) {}

AppState AppStateStore::load()
{
    auto repos = createSqliteRepositoryBundle(db_);
    core::application::AppStateManager mgr(std::move(repos));
    return mgr.load();
}

AppStateStoreResult AppStateStore::save(const AppState& state)
{
    AppStateStoreResult result;

    auto repos = createSqliteRepositoryBundle(db_);
    SqliteTransaction tx(db_->handle());

    const auto existingTransactionIds = collectRepositoryIds(
        repos.transactions, [](const auto& repo) { return repo.getTransactions(); });
    std::unordered_map<std::string, std::string> transactionStatementById;
    if (repos.transactions) {
        for (const auto& tx : repos.transactions->getTransactions()) {
            if (!tx || tx->id.empty()) continue;
            transactionStatementById.emplace(tx->id, tx->statementId);
        }
    }
    const auto existingStatementIds = collectRepositoryIds(
        repos.statements, [](const auto& repo) { return repo.getStatements(); });
    const auto existingContractIds = collectRepositoryIds(
        repos.contracts, [](const auto& repo) { return repo.getContracts(); });
    const auto existingActorIds = collectRepositoryIds(
        repos.actors, [](const auto& repo) { return repo.getActors(); });
    const auto existingPropertyIds = collectRepositoryIds(
        repos.properties, [](const auto& repo) { return repo.getProperties(); });
    const auto existingAnalysisIds = collectRepositoryIds(
        repos.analyses, [](const auto& repo) { return repo.getAnalyses(); });
    const auto existingAnnualIds = collectRepositoryIds(
        repos.annuals, [](const auto& repo) { return repo.getAnnuals(); });

    const auto newTransactionIds = collectIds(state.transactions);
    const auto newStatementIds = collectIds(state.statements);
    const auto newContractIds = collectIds(state.contracts);
    const auto newActorIds = collectIds(state.actors);
    const auto newPropertyIds = collectIds(state.properties);
    const auto newAnalysisIds = collectIds(state.analyses);
    const auto newAnnualIds = collectIds(state.annuals);

    deleteStaleIds(repos.statements,
                   existingStatementIds,
                   newStatementIds,
                   [](auto& repo, const std::string& id) { repo.removeStatement(id); },
                   &result.impact.deletedStatementIds);

    std::unordered_set<std::string> deletedStatementIds(result.impact.deletedStatementIds.begin(),
                                                        result.impact.deletedStatementIds.end());
    if (repos.transactions) {
        for (const auto& id : existingTransactionIds) {
            if (newTransactionIds.contains(id)) continue;
            const auto txIt = transactionStatementById.find(id);
            if (txIt != transactionStatementById.end() && deletedStatementIds.contains(txIt->second)) {
                continue;
            }
            repos.transactions->removeTransaction(id);
            result.impact.deletedTransactionIds.push_back(id);
        }
    }
    deleteStaleIds(repos.contracts,
                   existingContractIds,
                   newContractIds,
                   [](auto& repo, const std::string& id) { repo.removeContract(id); },
                   &result.impact.deletedContractIds);
    deleteStaleIds(repos.actors,
                   existingActorIds,
                   newActorIds,
                   [](auto& repo, const std::string& id) { repo.removeActor(id); },
                   &result.impact.deletedActorIds);
    deleteStaleIds(repos.properties,
                   existingPropertyIds,
                   newPropertyIds,
                   [](auto& repo, const std::string& id) { repo.removeProperty(id); },
                   &result.impact.deletedPropertyIds);
    deleteStaleIds(repos.analyses,
                   existingAnalysisIds,
                   newAnalysisIds,
                   [](auto& repo, const std::string& id) { repo.removeAnalysis(id); });
    deleteStaleIds(repos.annuals,
                   existingAnnualIds,
                   newAnnualIds,
                   [](auto& repo, const std::string& id) { repo.removeAnnual(id); });

    core::application::AppStateManager mgr(std::move(repos));
    mgr.save(state);

    tx.commit();
    return result;
}
