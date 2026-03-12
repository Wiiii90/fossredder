#include "core/application/WorkspaceSession.h"
#include "core/errors/ErrorReporterRegistry.h"

namespace core::application {

WorkspaceSession::WorkspaceSession(std::unique_ptr<core::storage::IStorageManager> sm)
    : storageManager_(std::move(sm))
{
    setDeletionImpactCallback({});
}

const std::string& WorkspaceSession::currentPath() const noexcept
{
    return storageManager_ ? storageManager_->currentPath() : emptyPath_;
}

void WorkspaceSession::setStateChangedCallback(StateChanged cb)  { onStateChanged_ = std::move(cb); }
void WorkspaceSession::setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> r) { errorReporter_ = std::move(r); }

void WorkspaceSession::setRepoFactory(core::storage::IStorageManager::RepoFactory f) {
    if (storageManager_) storageManager_->setRepoFactory(std::move(f));
}
void WorkspaceSession::setAtomicStoreSave(core::storage::IStorageManager::AtomicStoreSave f) {
    if (storageManager_) storageManager_->setAtomicStoreSave(std::move(f));
}
void WorkspaceSession::setAtomicStoreLoad(core::storage::IStorageManager::AtomicStoreLoad f) {
    if (storageManager_) storageManager_->setAtomicStoreLoad(std::move(f));
}
void WorkspaceSession::setDeletionImpactCallback(core::storage::IStorageManager::DeletionImpactCallback cb) {
    onDeletionImpact_ = std::move(cb);
    if (!storageManager_) return;
    storageManager_->setDeletionImpactCallback([this](const DeletionImpact& impact) {
        try { if (onDeletionImpact_) onDeletionImpact_(impact); }
        catch (...) { reportException(core::errors::ErrorSeverity::Error, "WorkspaceSession::setDeletionImpactCallback", std::current_exception()); }
    });
}

void WorkspaceSession::openLatest() {
    if (!storageManager_) return;
    if (auto p = storageManager_->loadLatestPath()) { state_ = storageManager_->loadFrom(*p); notifyState(); }
}
void WorkspaceSession::newFile(const std::string& path) {
    if (!storageManager_) return;
    storageManager_->createNew(path); state_ = AppState{}; notifyState();
}
void WorkspaceSession::openFile(const std::string& path) {
    if (!storageManager_) return;
    state_ = storageManager_->loadFrom(path); notifyState();
}
void WorkspaceSession::saveFile()                          { if (storageManager_) storageManager_->save(state_); }
void WorkspaceSession::saveFileAs(const std::string& path) { if (storageManager_) storageManager_->saveAs(path, state_); }

void WorkspaceSession::commit() {
    if (storageManager_) {
        try {
            const std::string& p = storageManager_->currentPath();
            if (!p.empty()) storageManager_->save(state_);
        } catch (...) { reportException(core::errors::ErrorSeverity::Error, "WorkspaceSession::commit", std::current_exception()); }
    }
    notifyState();
}

void WorkspaceSession::notifyState() {
    try { if (onStateChanged_) onStateChanged_(state_); }
    catch (...) { reportException(core::errors::ErrorSeverity::Error, "WorkspaceSession::notifyState", std::current_exception()); }
}

void WorkspaceSession::reportException(core::errors::ErrorSeverity sev, const char* origin, std::exception_ptr ex) const {
    if (errorReporter_) errorReporter_->reportException(sev, origin, ex);
    else core::errors::reportException(sev, origin, ex);
}

} // namespace core::application
