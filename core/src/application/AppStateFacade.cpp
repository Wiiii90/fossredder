/**
 * @file core/src/application/AppStateFacade.cpp
 * @brief Implements the application-facing facade for editable app state and workspace persistence.
 */

#include "core/pch.h"
#include "core/application/AppStateFacade.h"

#include "core/application/DraftFinalizer.h"
#include "core/application/WorkspaceSession.h"

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

std::string AppStateFacade::addAnalysis(const std::string& name, const std::string& type, const std::string& configJson, const std::string& filterSpec)
{
    return commitCreated(*this, catalog_.addAnalysis(mutableState(), {name, type, configJson, filterSpec}));
}

void AppStateFacade::updateAnalysis(const std::string& id, const std::string& name, const std::string& type, const std::string& configJson, const std::string& filterSpec)
{
    commitIfChanged(*this, catalog_.updateAnalysis(mutableState(), id, {name, type, configJson, filterSpec}));
}

void AppStateFacade::deleteAnalysis(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteAnalysis(mutableState(), id));
}

std::string AppStateFacade::addAnnual(int year)
{
    return commitCreated(*this, catalog_.addAnnual(mutableState(), year));
}

void AppStateFacade::updateAnnual(const std::string& id, int year)
{
    commitIfChanged(*this, catalog_.updateAnnual(mutableState(), id, year));
}

void AppStateFacade::deleteAnnual(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteAnnual(mutableState(), id));
}

std::string AppStateFacade::finalizeStatementDraft(const DraftStatement& draft)
{
    return commitCreated(*this, DraftFinalizer::finalize(mutableState(), draft));
}

void AppStateFacade::commit()
{
    session_->commit();
}

} // namespace core::application
