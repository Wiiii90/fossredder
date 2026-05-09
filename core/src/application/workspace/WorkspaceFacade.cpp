/**
 * @file core/src/application/workspace/WorkspaceFacade.cpp
 * @brief Implements the application-facing facade for editable app state and workspace persistence.
 */

#include "core/pch.h"
#include "core/application/workspace/WorkspaceFacade.h"

#include "core/application/import/draft/DraftFinalizer.h"
#include "core/application/workspace/WorkspaceDraftService.h"
#include "core/application/workspace/WorkspaceSession.h"
#include "../../utils/StableId.h"

#include <algorithm>

namespace {

std::string commitCreated(core::application::WorkspaceFacade& facade, std::string id)
{
    if (id.empty()) {
        return {};
    }

    facade.commit();
    return id;
}

void commitIfChanged(core::application::WorkspaceFacade& facade, bool changed)
{
    if (changed) {
        facade.commit();
    }
}

} // namespace

namespace core::application {

WorkspaceFacade::WorkspaceFacade(std::unique_ptr<core::ports::storage::IStorageManager> storageManager)
    : session_(std::make_unique<WorkspaceSession>(std::move(storageManager)))
{
}

WorkspaceFacade::~WorkspaceFacade() = default;

WorkspaceFacade::WorkspaceFacade(WorkspaceFacade&&) noexcept = default;

WorkspaceFacade& WorkspaceFacade::operator=(WorkspaceFacade&&) noexcept = default;

const WorkspaceState& WorkspaceFacade::state() const noexcept
{
    return session_->state();
}

const std::string& WorkspaceFacade::currentPath() const noexcept
{
    return session_->currentPath();
}

core::ports::presenters::WorkspacePresentation WorkspaceFacade::presentWorkspace() const
{
    const auto& path = currentPath();
    return {
        path,
        !path.empty()
    };
}

WorkspaceState& WorkspaceFacade::mutableState() noexcept
{
    return session_->mutableState();
}

void WorkspaceFacade::notifyState()
{
    session_->notifyState();
}

void WorkspaceFacade::setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter)
{
    session_->setErrorReporter(std::move(reporter));
}

void WorkspaceFacade::setStateChangedCallback(StateChanged cb)
{
    session_->setStateChangedCallback(std::move(cb));
}

void WorkspaceFacade::setAtomicStoreSave(core::ports::storage::IStorageManager::AtomicStoreSave saveFn)
{
    session_->setAtomicStoreSave(std::move(saveFn));
}

void WorkspaceFacade::setAtomicStoreLoad(core::ports::storage::IStorageManager::AtomicStoreLoad loadFn)
{
    session_->setAtomicStoreLoad(std::move(loadFn));
}

void WorkspaceFacade::setDeletionImpactCallback(core::ports::storage::IStorageManager::DeletionImpactCallback cb)
{
    session_->setDeletionImpactCallback(std::move(cb));
}

void WorkspaceFacade::openLatest()
{
    session_->openLatest();
}

void WorkspaceFacade::newFile(const std::string& path)
{
    session_->newFile(path);
}

void WorkspaceFacade::openFile(const std::string& path)
{
    session_->openFile(path);
}

void WorkspaceFacade::saveFile()
{
    session_->saveFile();
}

void WorkspaceFacade::saveFileAs(const std::string& path)
{
    session_->saveFileAs(path);
}

std::string WorkspaceFacade::addActor(const std::string& name,
                                      const std::vector<core::domain::Alias>& aliases)
{
    return commitCreated(*this, catalog_.addActor(mutableState(), {name, aliases}));
}

void WorkspaceFacade::updateActor(const std::string& id,
                                 const std::string& name,
                                 const std::vector<core::domain::Alias>& aliases)
{
    commitIfChanged(*this, catalog_.updateActor(mutableState(), id, {name, aliases}));
}

void WorkspaceFacade::deleteActor(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteActor(mutableState(), id));
}

std::string WorkspaceFacade::addProperty(const std::string& name,
                                        const std::vector<core::domain::Alias>& aliases)
{
    return commitCreated(*this, catalog_.addProperty(mutableState(), {name, aliases}));
}

void WorkspaceFacade::updateProperty(const std::string& id,
                                    const std::string& name,
                                    const std::vector<core::domain::Alias>& aliases)
{
    commitIfChanged(*this, catalog_.updateProperty(mutableState(), id, {name, aliases}));
}

void WorkspaceFacade::deleteProperty(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteProperty(mutableState(), id));
}

std::string WorkspaceFacade::addContract(const std::string& name,
                                        const std::string& type,
                                        const std::vector<std::string>& actorIds,
                                        const std::vector<std::string>& propertyIds,
                                        const std::vector<core::domain::Alias>& aliases)
{
    return commitCreated(*this, catalog_.addContract(mutableState(), {name, type, actorIds, propertyIds, aliases}));
}

void WorkspaceFacade::updateContract(const std::string& id,
                                    const std::string& name,
                                    const std::string& type,
                                    const std::vector<std::string>& actorIds,
                                    const std::vector<std::string>& propertyIds,
                                    const std::vector<core::domain::Alias>& aliases)
{
    commitIfChanged(*this, catalog_.updateContract(mutableState(), id, {name, type, actorIds, propertyIds, aliases}));
}

void WorkspaceFacade::deleteContract(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteContract(mutableState(), id));
}

std::vector<std::string> WorkspaceFacade::contractTypes() const
{
    return catalog_.contractTypes(state());
}

std::string WorkspaceFacade::addStatement(const std::string& name)
{
    return commitCreated(*this, catalog_.addStatement(mutableState(), name));
}

void WorkspaceFacade::updateStatement(const std::string& id, const std::string& name)
{
    commitIfChanged(*this, catalog_.updateStatement(mutableState(), id, name));
}

void WorkspaceFacade::deleteStatement(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteStatement(mutableState(), id));
}

std::string WorkspaceFacade::addTransaction(const std::string& name,
                                           const std::string& bookingDate,
                                           double amount,
                                           const std::string& statementId,
                                           Transaction::Status status,
                                           const std::string& actorId,
                                           bool allocatable,
                                           const std::vector<std::string>& propertyIds)
{
    TransactionInput input;
    input.name = name;
    input.bookingDate = bookingDate;
    input.amount = amount;
    input.statementId = statementId;
    input.status = status;
    input.actorId = actorId;
    input.allocatable = allocatable;
    input.propertyIds = propertyIds;
    return commitCreated(*this, catalog_.addTransaction(mutableState(), input));
}

void WorkspaceFacade::updateTransaction(const std::string& id,
                                       const std::string& name,
                                       const std::string& bookingDate,
                                       double amount,
                                       const std::string& statementId,
                                       Transaction::Status status,
                                       const std::string& actorId,
                                       bool allocatable,
                                       const std::vector<std::string>& propertyIds)
{
    TransactionInput input;
    input.name = name;
    input.bookingDate = bookingDate;
    input.amount = amount;
    input.statementId = statementId;
    input.status = status;
    input.actorId = actorId;
    input.allocatable = allocatable;
    input.propertyIds = propertyIds;
    commitIfChanged(*this, catalog_.updateTransaction(mutableState(), id, input));
}

void WorkspaceFacade::deleteTransaction(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteTransaction(mutableState(), id));
}

std::string WorkspaceFacade::addAnalysis(const std::string& name,
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

void WorkspaceFacade::updateAnalysis(const std::string& id,
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

void WorkspaceFacade::deleteAnalysis(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteAnalysis(mutableState(), id));
}

std::string WorkspaceFacade::addAnnual(const std::string& name,
                                      int year,
                                      const std::vector<std::string>& assignedAnalysisIds)
{
    return commitCreated(*this, catalog_.addAnnual(mutableState(), name, year, assignedAnalysisIds));
}

void WorkspaceFacade::updateAnnual(const std::string& id,
                                  const std::string& name,
                                  int year,
                                  const std::vector<std::string>& assignedAnalysisIds)
{
    commitIfChanged(*this,
                    catalog_.updateAnnual(mutableState(), id, name, year, assignedAnalysisIds));
}

void WorkspaceFacade::deleteAnnual(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteAnnual(mutableState(), id));
}

std::string WorkspaceFacade::finalizeStatementDraft(const core::domain::StatementDraft& draft)
{
    return commitCreated(*this, DraftFinalizer::finalize(mutableState(), draft));
}

void WorkspaceFacade::saveStatementDraft(const core::domain::StatementDraft& draft)
{
    workspace::WorkspaceDraftService::saveStatementDraft(mutableState(), draft);
    commit();
}

void WorkspaceFacade::clearStatementDraft(const std::string& draftId)
{
    workspace::WorkspaceDraftService::clearStatementDraft(mutableState(), draftId);
    commit();
}

std::optional<core::domain::StatementDraft> WorkspaceFacade::loadStatementDraft(const std::string& draftId) const
{
    return workspace::WorkspaceDraftService::loadStatementDraft(state(), draftId);
}

void WorkspaceFacade::setImportLogs(const std::vector<core::domain::ImportLog>& logs)
{
    workspace::WorkspaceLogService::setImportLogs(mutableState(), logs);
    commit();
}

std::vector<std::shared_ptr<core::domain::ImportLog>> WorkspaceFacade::importLogs() const
{
    return workspace::WorkspaceLogService::importLogs(state());
}

void WorkspaceFacade::setExportLogs(const std::vector<core::domain::ExportLog>& logs)
{
    workspace::WorkspaceLogService::setExportLogs(mutableState(), logs);
    commit();
}

std::vector<std::shared_ptr<core::domain::ExportLog>> WorkspaceFacade::exportLogs() const
{
    return workspace::WorkspaceLogService::exportLogs(state());
}

void WorkspaceFacade::commit()
{
    session_->commit();
}

} // namespace core::application
