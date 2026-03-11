/**
 * @file core/src/controllers/AppStateController.cpp
 * @brief Implementation of AppStateController.
 *
 * Implements the runtime behaviour of the AppStateController which manages the
 * in-memory AppState and delegates persistence to an IStorageManager.
 */

#include "core/pch.h"
#include "core/controllers/AppStateController.h"
#include "core/application/DraftFinalizer.h"

namespace {

std::string commitCreated(core::controllers::AppStateController& controller, std::string id)
{
    if (id.empty()) return {};
    controller.commit();
    return id;
}

void commitIfChanged(core::controllers::AppStateController& controller, bool changed)
{
    if (changed) controller.commit();
}

}

namespace core::controllers {

AppStateController::AppStateController(std::unique_ptr<core::storage::IStorageManager> storageManager)
    : session_(std::make_unique<core::application::WorkspaceSession>(std::move(storageManager))) {
}

void AppStateController::notifyState() {
    session_->notifyState();
}

void AppStateController::setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter)
{
    session_->setErrorReporter(std::move(reporter));
}

void AppStateController::setStateChangedCallback(StateChanged cb) {
    session_->setStateChangedCallback(std::move(cb));
}

void AppStateController::setRepoFactory(core::storage::IStorageManager::RepoFactory factory) {
    session_->setRepoFactory(std::move(factory));
}

void AppStateController::setAtomicStoreSave(core::storage::IStorageManager::AtomicStoreSave saveFn) {
    session_->setAtomicStoreSave(std::move(saveFn));
}

void AppStateController::setAtomicStoreLoad(core::storage::IStorageManager::AtomicStoreLoad loadFn) {
    session_->setAtomicStoreLoad(std::move(loadFn));
}

void AppStateController::setDeletionImpactCallback(core::storage::IStorageManager::DeletionImpactCallback cb) {
    session_->setDeletionImpactCallback(std::move(cb));
}

void AppStateController::openLatest() {
    session_->openLatest();
}

void AppStateController::newFile(const std::string& path) {
    session_->newFile(path);
}

void AppStateController::openFile(const std::string& path) {
    session_->openFile(path);
}

void AppStateController::saveFile() {
    session_->saveFile();
}

void AppStateController::saveFileAs(const std::string& path) {
    session_->saveFileAs(path);
}

std::string AppStateController::addActor(const std::string& name, const std::string& type, const std::string& description)
{
    return commitCreated(*this, catalog_.addActor(mutableState(), name, type, description));
}

void AppStateController::updateActor(const std::string& id, const std::string& name, const std::string& type, const std::string& description)
{
    commitIfChanged(*this, catalog_.updateActor(mutableState(), id, name, type, description));
}

void AppStateController::deleteActor(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteActor(mutableState(), id));
}

std::string AppStateController::addProperty(const std::string& name, const std::string& address, const std::string& description)
{
    return commitCreated(*this, catalog_.addProperty(mutableState(), name, address, description));
}

void AppStateController::updateProperty(const std::string& id, const std::string& name, const std::string& address, const std::string& description)
{
    commitIfChanged(*this, catalog_.updateProperty(mutableState(), id, name, address, description));
}

void AppStateController::deleteProperty(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteProperty(mutableState(), id));
}

std::string AppStateController::addContract(const std::string& name, const std::string& type, const std::string& description,
                                            const std::vector<std::string>& actorIds, const std::vector<std::string>& propertyIds)
{
    return commitCreated(*this, catalog_.addContract(mutableState(), name, type, description, actorIds, propertyIds));
}

void AppStateController::updateContract(const std::string& id, const std::string& name, const std::string& type, const std::string& description,
                                        const std::vector<std::string>& actorIds, const std::vector<std::string>& propertyIds)
{
    commitIfChanged(*this, catalog_.updateContract(mutableState(), id, name, type, description, actorIds, propertyIds));
}

void AppStateController::deleteContract(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteContract(mutableState(), id));
}

std::vector<std::string> AppStateController::contractTypes() const
{
    return catalog_.contractTypes(state());
}

std::string AppStateController::addStatement(const std::string& name)
{
    return commitCreated(*this, catalog_.addStatement(mutableState(), name));
}

void AppStateController::updateStatement(const std::string& id, const std::string& name)
{
    commitIfChanged(*this, catalog_.updateStatement(mutableState(), id, name));
}

void AppStateController::deleteStatement(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteStatement(mutableState(), id));
}

std::string AppStateController::addTransaction(const std::string& name,
                                               const std::string& bookingDate,
                                               double amount,
                                               const std::string& description,
                                               const std::string& statementId,
                                               Transaction::Status status,
                                               const std::string& actorId,
                                               bool allocatable,
                                               const std::vector<std::string>& propertyIds)
{
    return commitCreated(*this, catalog_.addTransaction(mutableState(), name, bookingDate, amount, description, statementId, status, actorId, allocatable, propertyIds));
}

void AppStateController::updateTransaction(const std::string& id,
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
    commitIfChanged(*this, catalog_.updateTransaction(mutableState(), id, name, bookingDate, amount, description, statementId, status, actorId, allocatable, propertyIds));
}

void AppStateController::deleteTransaction(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteTransaction(mutableState(), id));
}

std::string AppStateController::addAnalysis(const std::string& name, const std::string& type, const std::string& configJson, const std::string& filterSpec)
{
    return commitCreated(*this, catalog_.addAnalysis(mutableState(), name, type, configJson, filterSpec));
}

void AppStateController::updateAnalysis(const std::string& id, const std::string& name, const std::string& type, const std::string& configJson, const std::string& filterSpec)
{
    commitIfChanged(*this, catalog_.updateAnalysis(mutableState(), id, name, type, configJson, filterSpec));
}

void AppStateController::deleteAnalysis(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteAnalysis(mutableState(), id));
}

std::string AppStateController::addAnnual(int year)
{
    return commitCreated(*this, catalog_.addAnnual(mutableState(), year));
}

void AppStateController::updateAnnual(const std::string& id, int year)
{
    commitIfChanged(*this, catalog_.updateAnnual(mutableState(), id, year));
}

void AppStateController::deleteAnnual(const std::string& id)
{
    commitIfChanged(*this, catalog_.deleteAnnual(mutableState(), id));
}

std::string AppStateController::finalizeStatementDraft(const DraftStatement& draft)
{
    return commitCreated(*this, core::application::DraftFinalizer::finalize(mutableState(), draft));
}

void AppStateController::commit() {
    session_->commit();
}

}
