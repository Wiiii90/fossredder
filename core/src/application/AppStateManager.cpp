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

#include "core/models/AppState.h"
#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/Contract.h"
#include "core/repositories/IActorRepository.h"
#include "core/repositories/IAnalysisRepository.h"
#include "core/repositories/IAnnualRepository.h"
#include "core/repositories/IContractRepository.h"
#include "core/repositories/IImportLogRepository.h"
#include "core/repositories/IPropertyRepository.h"
#include "core/repositories/IStatementRepository.h"
#include "core/repositories/IStatementDraftRepository.h"
#include "core/repositories/ITransactionRepository.h"
#include "core/repositories/ITransactionDraftRepository.h"
#include "core/storage/RepositoryBundle.h"

#include "core/models/Property.h"
#include "core/models/ImportLog.h"
#include "core/models/Statement.h"
#include "core/models/StatementDraft.h"
#include "core/models/Transaction.h"
#include "core/models/TransactionDraft.h"

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
    if (impl_->repos.importLogs) {
        impl_->repos.importLogs->clearImportLogs();
        for (const auto& item : projected.importLogs) {
            if (!item) continue;
            impl_->repos.importLogs->upsertImportLog(item);
        }
    }

    if (impl_->repos.statementDrafts) {
        impl_->repos.statementDrafts->clearStatementDrafts();
        for (const auto& item : projected.statementDrafts) {
            if (!item) continue;
            impl_->repos.statementDrafts->upsertStatementDraft(item);
        }
    }

    if (impl_->repos.transactionDrafts) {
        std::vector<std::shared_ptr<core::domain::TransactionDraft>> transactionDrafts;
        transactionDrafts.reserve(projected.transactionDrafts.size());
        for (const auto& item : projected.transactionDrafts) {
            if (!item) continue;
            transactionDrafts.push_back(item);
        }

        impl_->repos.transactionDrafts->clearTransactionDrafts();
        for (const auto& draft : transactionDrafts) {
            impl_->repos.transactionDrafts->upsertTransactionDraft(draft);
        }
    }
}

void AppStateManager::setStrictValidation(bool enabled) noexcept
{
    impl_->strictValidation = enabled;
}

}
