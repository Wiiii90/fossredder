/**
 * @file core/src/application/AppStateManager.cpp
 * @brief Implementation of AppStateManager.
 *
 * Implements loading, saving, rehydration and validation of the application
 * state using the provided repository abstractions.
 */

#include "core/pch.h"

#include "core/application/AppStateManager.h"
#include "core/application/StateHydrator.h"
#include "core/application/StateProjector.h"

#include "core/repositories/IActorRepository.h"
#include "core/repositories/IAnalysisRepository.h"
#include "core/repositories/IAnnualRepository.h"
#include "core/repositories/IContractRepository.h"
#include "core/repositories/IImportLogRepository.h"
#include "core/repositories/IExportLogRepository.h"
#include "core/repositories/IPropertyRepository.h"
#include "core/repositories/IStatementRepository.h"
#include "core/repositories/IStatementDraftRepository.h"
#include "core/repositories/ITransactionRepository.h"
#include "core/repositories/ITransactionDraftRepository.h"
#include "core/storage/RepositoryBundle.h"

namespace core::application {

namespace {

/** @brief Loads a collection from a repository when the repository is present. */
template <typename RepoPtr, typename LoadFn>
auto loadCollection(const RepoPtr& repo, LoadFn&& load)
{
    using Collection = std::decay_t<decltype(load(*repo))>;
    if (!repo) return Collection{};
    return load(*repo);
}

/** @brief Upserts a collection into a repository while skipping null items. */
template <typename RepoPtr, typename Collection, typename UpsertFn>
void upsertCollection(const RepoPtr& repo, const Collection& items, UpsertFn&& upsert)
{
    if (!repo) return;
    for (const auto& item : items) {
        if (!item) continue;
        upsert(*repo, item);
    }
}

/** @brief Replaces all items in a repository with the provided collection. */
template <typename RepoPtr, typename Collection, typename ClearFn, typename UpsertFn>
void replaceCollection(const RepoPtr& repo, const Collection& items, ClearFn&& clear, UpsertFn&& upsert)
{
    if (!repo) return;

    clear(*repo);
    for (const auto& item : items) {
        if (!item) continue;
        upsert(*repo, item);
    }
}

}

class AppStateManager::Impl {
public:
    explicit Impl(Repositories repositories)
        : repos(std::move(repositories))
    {
    }

    Repositories repos;
    bool strictValidation = false;
};

AppStateManager::AppStateManager(Repositories repos)
    : impl_(std::make_unique<Impl>(std::move(repos)))
{
}

AppStateManager::~AppStateManager() = default;

AppStateManager::AppStateManager(AppStateManager&&) noexcept = default;

AppStateManager& AppStateManager::operator=(AppStateManager&&) noexcept = default;

AppState AppStateManager::load() {
    AppState state;

    state.actors = loadCollection(impl_->repos.actors, [](const auto& repo) { return repo.getActors(); });
    state.properties = loadCollection(impl_->repos.properties, [](const auto& repo) { return repo.getProperties(); });
    state.contracts = loadCollection(impl_->repos.contracts, [](const auto& repo) { return repo.getContracts(); });
    state.statements = loadCollection(impl_->repos.statements, [](const auto& repo) { return repo.getStatements(); });
    state.transactions = loadCollection(impl_->repos.transactions, [](const auto& repo) { return repo.getTransactions(); });
    state.analyses = loadCollection(impl_->repos.analyses, [](const auto& repo) { return repo.getAnalyses(); });
    state.annuals = loadCollection(impl_->repos.annuals, [](const auto& repo) { return repo.getAnnuals(); });
    state.importLogs = loadCollection(impl_->repos.importLogs, [](const auto& repo) { return repo.getImportLogs(); });
    state.exportLogs = loadCollection(impl_->repos.exportLogs, [](const auto& repo) { return repo.getExportLogs(); });

    state.statementDrafts = loadCollection(impl_->repos.statementDrafts, [](const auto& repo) { return repo.getStatementDrafts(); });

    if (impl_->repos.transactionDrafts) {
        state.transactionDrafts = impl_->repos.transactionDrafts->getTransactionDrafts();
    }

    StateHydrator::rehydrate(state);
    StateHydrator::validate(state, impl_->strictValidation);

    return state;
}

void AppStateManager::save(const AppState& state) {
    const AppState projected = StateProjector::prepareForSave(state);
    StateHydrator::validate(projected, impl_->strictValidation);

    upsertCollection(impl_->repos.actors, projected.actors, [](auto& repo, const auto& actor) { repo.upsertActor(actor); });
    upsertCollection(impl_->repos.properties, projected.properties, [](auto& repo, const auto& property) { repo.upsertProperty(property); });
    upsertCollection(impl_->repos.contracts, projected.contracts, [](auto& repo, const auto& contract) { repo.upsertContract(contract); });
    upsertCollection(impl_->repos.statements, projected.statements, [](auto& repo, const auto& statement) { repo.upsertStatement(statement); });
    upsertCollection(impl_->repos.transactions, projected.transactions, [](auto& repo, const auto& transaction) { repo.upsertTransaction(transaction); });
    upsertCollection(impl_->repos.analyses, projected.analyses, [](auto& repo, const auto& analysis) { repo.upsertAnalysis(analysis); });
    upsertCollection(impl_->repos.annuals, projected.annuals, [](auto& repo, const auto& annual) { repo.upsertAnnual(annual); });
    replaceCollection(impl_->repos.importLogs,
                      projected.importLogs,
                      [](auto& repo) { repo.clearImportLogs(); },
                      [](auto& repo, const auto& item) { repo.upsertImportLog(item); });
    replaceCollection(impl_->repos.exportLogs,
                      projected.exportLogs,
                      [](auto& repo) { repo.clearExportLogs(); },
                      [](auto& repo, const auto& item) { repo.upsertExportLog(item); });
    replaceCollection(impl_->repos.statementDrafts,
                      projected.statementDrafts,
                      [](auto& repo) { repo.clearStatementDrafts(); },
                      [](auto& repo, const auto& item) { repo.upsertStatementDraft(item); });
    replaceCollection(impl_->repos.transactionDrafts,
                      projected.transactionDrafts,
                      [](auto& repo) { repo.clearTransactionDrafts(); },
                      [](auto& repo, const auto& item) { repo.upsertTransactionDraft(item); });
}

void AppStateManager::setStrictValidation(bool enabled) noexcept
{
    impl_->strictValidation = enabled;
}

}
