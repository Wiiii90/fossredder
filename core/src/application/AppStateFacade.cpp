/**
 * @file core/src/application/AppStateFacade.cpp
 * @brief Implements the application-facing facade for editable app state and workspace persistence.
 */

#include "core/pch.h"
#include "core/application/AppStateFacade.h"

#include "core/application/DraftFinalizer.h"
#include "core/application/WorkspaceSession.h"
#include "../utils/StableId.h"

#include <algorithm>

namespace {

std::string commitCreated(core::application::AppStateFacade& facade, std::string id)
{
    if (id.empty()) {
        return {};
    }

    facade.commit();
    return id;
}

void commitIfChanged(core::application::AppStateFacade& facade, bool changed)
{
    if (changed) {
        facade.commit();
    }
}

} // namespace

namespace core::application {

AppStateFacade::AppStateFacade(std::unique_ptr<core::storage::IStorageManager> storageManager)
    : session_(std::make_unique<WorkspaceSession>(std::move(storageManager)))
{
}

AppStateFacade::~AppStateFacade() = default;

AppStateFacade::AppStateFacade(AppStateFacade&&) noexcept = default;

AppStateFacade& AppStateFacade::operator=(AppStateFacade&&) noexcept = default;

const AppState& AppStateFacade::state() const noexcept
{
    return session_->state();
}

const std::string& AppStateFacade::currentPath() const noexcept
{
    return session_->currentPath();
}

AppState& AppStateFacade::mutableState() noexcept
{
    return session_->mutableState();
}

void AppStateFacade::notifyState()
{
    session_->notifyState();
}

void AppStateFacade::setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter)
{
    session_->setErrorReporter(std::move(reporter));
}

void AppStateFacade::setStateChangedCallback(StateChanged cb)
{
    session_->setStateChangedCallback(std::move(cb));
}

void AppStateFacade::setRepoFactory(core::storage::IStorageManager::RepoFactory factory)
{
    session_->setRepoFactory(std::move(factory));
}

void AppStateFacade::setAtomicStoreSave(core::storage::IStorageManager::AtomicStoreSave saveFn)
{
    session_->setAtomicStoreSave(std::move(saveFn));
}

void AppStateFacade::setAtomicStoreLoad(core::storage::IStorageManager::AtomicStoreLoad loadFn)
{
    session_->setAtomicStoreLoad(std::move(loadFn));
}

void AppStateFacade::setDeletionImpactCallback(core::storage::IStorageManager::DeletionImpactCallback cb)
{
    session_->setDeletionImpactCallback(std::move(cb));
}

void AppStateFacade::openLatest()
{
    session_->openLatest();
}

void AppStateFacade::newFile(const std::string& path)
{
    session_->newFile(path);
}

void AppStateFacade::openFile(const std::string& path)
{
    session_->openFile(path);
}

void AppStateFacade::saveFile()
{
    session_->saveFile();
}

void AppStateFacade::saveFileAs(const std::string& path)
{
    session_->saveFileAs(path);
}

std::string AppStateFacade::addActor(const std::string& name,
                                     const std::string& type,
                                     const std::string& description,
                                     const std::vector<std::string>& aliases)
{
    return commitCreated(*this, catalog_.addActor(mutableState(), {name, type, description, aliases}));
}

void AppStateFacade::updateActor(const std::string& id,
                                 const std::string& name,
                                 const std::string& type,
                                 const std::string& description,
                                 const std::vector<std::string>& aliases)
{
    commitIfChanged(*this, catalog_.updateActor(mutableState(), id, {name, type, description, aliases}));
}

void AppStateFacade::deleteActor(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteActor(mutableState(), id));
}

std::string AppStateFacade::addProperty(const std::string& name,
                                        const std::string& address,
                                        const std::string& description,
                                        const std::vector<std::string>& aliases)
{
    return commitCreated(*this, catalog_.addProperty(mutableState(), {name, address, description, aliases}));
}

void AppStateFacade::updateProperty(const std::string& id,
                                    const std::string& name,
                                    const std::string& address,
                                    const std::string& description,
                                    const std::vector<std::string>& aliases)
{
    commitIfChanged(*this, catalog_.updateProperty(mutableState(), id, {name, address, description, aliases}));
}

void AppStateFacade::deleteProperty(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteProperty(mutableState(), id));
}

std::string AppStateFacade::addContract(const std::string& name,
                                        const std::string& type,
                                        const std::string& description,
                                        const std::vector<std::string>& actorIds,
                                        const std::vector<std::string>& propertyIds,
                                        const std::vector<std::string>& aliases)
{
    return commitCreated(*this, catalog_.addContract(mutableState(), {name, type, description, actorIds, propertyIds, aliases}));
}

void AppStateFacade::updateContract(const std::string& id,
                                    const std::string& name,
                                    const std::string& type,
                                    const std::string& description,
                                    const std::vector<std::string>& actorIds,
                                    const std::vector<std::string>& propertyIds,
                                    const std::vector<std::string>& aliases)
{
    commitIfChanged(*this, catalog_.updateContract(mutableState(), id, {name, type, description, actorIds, propertyIds, aliases}));
}

void AppStateFacade::deleteContract(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteContract(mutableState(), id));
}

std::vector<std::string> AppStateFacade::contractTypes() const
{
    return catalog_.contractTypes(state());
}

std::string AppStateFacade::addStatement(const std::string& name)
{
    return commitCreated(*this, catalog_.addStatement(mutableState(), name));
}

void AppStateFacade::updateStatement(const std::string& id, const std::string& name)
{
    commitIfChanged(*this, catalog_.updateStatement(mutableState(), id, name));
}

void AppStateFacade::deleteStatement(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteStatement(mutableState(), id));
}

std::string AppStateFacade::addTransaction(const std::string& name,
                                           const std::string& bookingDate,
                                           double amount,
                                           const std::string& description,
                                           const std::string& statementId,
                                           Transaction::Status status,
                                           const std::string& actorId,
                                           bool allocatable,
                                           const std::vector<std::string>& propertyIds)
{
    return commitCreated(*this, catalog_.addTransaction(mutableState(), {
        name,
        bookingDate,
        amount,
        description,
        statementId,
        status,
        actorId,
        allocatable,
        propertyIds
    }));
}

void AppStateFacade::updateTransaction(const std::string& id,
                                       const std::string& name,
                                       const std::string& bookingDate,
                                       double amount,
                                       const std::string& description,
                                       const std::string& statementId,
                                       Transaction::Status status,
                                       const std::string& actorId,
                                       bool allocatable,
                                       const std::vector<std::string>& propertyIds)
{
    commitIfChanged(*this, catalog_.updateTransaction(mutableState(), id, {
        name,
        bookingDate,
        amount,
        description,
        statementId,
        status,
        actorId,
        allocatable,
        propertyIds
    }));
}

void AppStateFacade::deleteTransaction(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteTransaction(mutableState(), id));
}

std::string AppStateFacade::addAnalysis(const std::string& name,
                                        const std::string& type,
                                        const std::string& configJson,
                                        const std::string& filterSpec,
                                        const std::string& exportFormat,
                                        bool includeCalcAdjustments,
                                        const std::string& exportStateJson,
                                        const std::string& snapshotTransactionsJson)
{
    return commitCreated(*this, catalog_.addAnalysis(mutableState(), {
        name,
        type,
        configJson,
        filterSpec,
        exportFormat,
        includeCalcAdjustments,
        exportStateJson,
        snapshotTransactionsJson
    }));
}

void AppStateFacade::updateAnalysis(const std::string& id,
                                    const std::string& name,
                                    const std::string& type,
                                    const std::string& configJson,
                                    const std::string& filterSpec,
                                    const std::string& exportFormat,
                                    bool includeCalcAdjustments,
                                    const std::string& exportStateJson,
                                    const std::string& snapshotTransactionsJson)
{
    commitIfChanged(*this, catalog_.updateAnalysis(mutableState(), id, {
        name,
        type,
        configJson,
        filterSpec,
        exportFormat,
        includeCalcAdjustments,
        exportStateJson,
        snapshotTransactionsJson
    }));
}

void AppStateFacade::deleteAnalysis(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteAnalysis(mutableState(), id));
}

std::string AppStateFacade::addAnnual(const std::string& name,
                                      int year,
                                      const std::vector<std::string>& assignedAnalysisIds)
{
    return commitCreated(*this, catalog_.addAnnual(mutableState(), name, year, assignedAnalysisIds));
}

void AppStateFacade::updateAnnual(const std::string& id,
                                  const std::string& name,
                                  int year,
                                  const std::vector<std::string>& assignedAnalysisIds)
{
    commitIfChanged(*this,
                    catalog_.updateAnnual(mutableState(), id, name, year, assignedAnalysisIds));
}

void AppStateFacade::deleteAnnual(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteAnnual(mutableState(), id));
}

std::string AppStateFacade::finalizeStatementDraft(const core::domain::StatementDraft& draft)
{
    return commitCreated(*this, DraftFinalizer::finalize(mutableState(), draft));
}

void AppStateFacade::saveStatementDraft(const core::domain::StatementDraft& draft)
{
    auto& state = mutableState();
    auto statementDraft = std::make_shared<core::domain::StatementDraft>(draft);
    if (!statementDraft) {
        commit();
        return;
    }

    if (statementDraft->id.empty()) {
        statementDraft->id = core::utils::makeStableId();
    }

    auto statementIt = std::find_if(state.statementDrafts.begin(), state.statementDrafts.end(),
                                    [statementDraft](const auto& item) {
                                        return item && item->id == statementDraft->id;
                                    });
    if (statementIt == state.statementDrafts.end()) {
        state.statementDrafts.push_back(statementDraft);
    } else {
        *statementIt = statementDraft;
    }

    state.transactionDrafts.erase(
        std::remove_if(state.transactionDrafts.begin(), state.transactionDrafts.end(),
                       [statementDraft](const auto& tx) {
                           return tx && tx->statementDraftId == statementDraft->id;
                       }),
        state.transactionDrafts.end());

    for (std::size_t i = 0; i < statementDraft->transactions.size(); ++i) {
        auto tx = std::make_shared<core::domain::TransactionDraft>(statementDraft->transactions[i]);
        if (!tx) continue;
        if (tx->id.empty()) {
            tx->id = core::utils::makeStableId();
        }
        tx->statementDraftId = statementDraft->id;
        tx->position = static_cast<int>(i);
        state.transactionDrafts.push_back(std::move(tx));
    }

    commit();
}

void AppStateFacade::clearStatementDraft(const std::string& draftId)
{
    auto& state = mutableState();
    if (draftId.empty()) {
        state.statementDrafts.clear();
        state.transactionDrafts.clear();
        commit();
        return;
    }

    state.statementDrafts.erase(
        std::remove_if(state.statementDrafts.begin(), state.statementDrafts.end(),
                       [&draftId](const auto& draft) {
                           return draft && draft->id == draftId;
                       }),
        state.statementDrafts.end());

    state.transactionDrafts.erase(
        std::remove_if(state.transactionDrafts.begin(), state.transactionDrafts.end(),
                       [&draftId](const auto& tx) {
                           return tx && tx->statementDraftId == draftId;
                       }),
        state.transactionDrafts.end());
    commit();
}

std::optional<core::domain::StatementDraft> AppStateFacade::loadStatementDraft(const std::string& draftId) const
{
    const auto& state = this->state();
    if (state.statementDrafts.empty()) {
        return std::nullopt;
    }

    auto statementIt = state.statementDrafts.begin();
    if (!draftId.empty()) {
        statementIt = std::find_if(state.statementDrafts.begin(), state.statementDrafts.end(),
                                   [&draftId](const auto& draft) {
                                       return draft && draft->id == draftId;
                                   });
    }
    if (statementIt == state.statementDrafts.end() || !*statementIt) {
        return std::nullopt;
    }

    core::domain::StatementDraft out = **statementIt;
    out.transactions.clear();

    std::vector<std::shared_ptr<core::domain::TransactionDraft>> txDrafts;
    txDrafts.reserve(state.transactionDrafts.size());
    for (const auto& tx : state.transactionDrafts) {
        if (!tx) continue;
        if (tx->statementDraftId != out.id) continue;
        txDrafts.push_back(tx);
    }

    std::sort(txDrafts.begin(), txDrafts.end(), [](const auto& lhs, const auto& rhs) {
        if (!lhs) return false;
        if (!rhs) return true;
        return lhs->position < rhs->position;
    });

    out.transactions.reserve(txDrafts.size());
    for (const auto& tx : txDrafts) {
        if (!tx) continue;
        out.transactions.push_back(*tx);
    }

    return out;
}

void AppStateFacade::setImportLogs(const std::vector<core::domain::ImportLog>& logs)
{
    auto& state = mutableState();
    state.importLogs.clear();
    state.importLogs.reserve(logs.size());
    for (const auto& item : logs) {
        auto log = std::make_shared<core::domain::ImportLog>(item);
        if (!log) continue;
        if (log->id.empty()) {
            log->id = core::utils::makeStableId();
        }
        state.importLogs.push_back(std::move(log));
    }
    commit();
}

std::vector<std::shared_ptr<core::domain::ImportLog>> AppStateFacade::importLogs() const
{
    return state().importLogs;
}

void AppStateFacade::setExportLogs(const std::vector<core::domain::ExportLog>& logs)
{
    auto& state = mutableState();
    state.exportLogs.clear();
    state.exportLogs.reserve(logs.size());
    for (const auto& item : logs) {
        auto log = std::make_shared<core::domain::ExportLog>(item);
        if (!log) continue;
        if (log->id.empty()) {
            log->id = core::utils::makeStableId();
        }
        state.exportLogs.push_back(std::move(log));
    }
    commit();
}

std::vector<std::shared_ptr<core::domain::ExportLog>> AppStateFacade::exportLogs() const
{
    return state().exportLogs;
}

void AppStateFacade::commit()
{
    session_->commit();
}

} // namespace core::application
