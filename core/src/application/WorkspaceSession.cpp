#include "core/application/WorkspaceSession.h"

#include "core/errors/ErrorReporterRegistry.h"

namespace core::application {

WorkspaceSession::WorkspaceSession(std::unique_ptr<core::storage::IStorageManager> storageManager)
    : storageManager_(std::move(storageManager))
{
    setDeletionImpactCallback({});
}

void WorkspaceSession::setStateChangedCallback(StateChanged cb)
{
    onStateChanged_ = std::move(cb);
}

void WorkspaceSession::setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter)
{
    errorReporter_ = std::move(reporter);
}

void WorkspaceSession::setRepoFactory(core::storage::IStorageManager::RepoFactory factory)
{
    if (storageManager_) storageManager_->setRepoFactory(std::move(factory));
}

void WorkspaceSession::setAtomicStoreSave(core::storage::IStorageManager::AtomicStoreSave saveFn)
{
    if (storageManager_) storageManager_->setAtomicStoreSave(std::move(saveFn));
}

void WorkspaceSession::setAtomicStoreLoad(core::storage::IStorageManager::AtomicStoreLoad loadFn)
{
    if (storageManager_) storageManager_->setAtomicStoreLoad(std::move(loadFn));
}

void WorkspaceSession::setDeletionImpactCallback(core::storage::IStorageManager::DeletionImpactCallback cb)
{
    onDeletionImpact_ = std::move(cb);
    if (!storageManager_) return;

    storageManager_->setDeletionImpactCallback([this](const DeletionImpact& impact) {
        try {
            if (onDeletionImpact_) onDeletionImpact_(impact);
        } catch (...) {
            reportException(core::errors::ErrorSeverity::Error, "WorkspaceSession::setDeletionImpactCallback", std::current_exception());
        }
    });
}

void WorkspaceSession::openLatest()
{
    if (!storageManager_) return;
    if (auto latest = storageManager_->loadLatestPath()) {
        state_ = storageManager_->loadFrom(*latest);
        notify();
    }
}

void WorkspaceSession::newFile(const std::string& path)
{
    if (!storageManager_) return;
    storageManager_->createNew(path);
    state_ = AppState{};
    notify();
}

void WorkspaceSession::openFile(const std::string& path)
{
    if (!storageManager_) return;
    state_ = storageManager_->loadFrom(path);
    notify();
}

void WorkspaceSession::saveFile()
{
    if (!storageManager_) return;
    storageManager_->save(state_);
}

void WorkspaceSession::saveFileAs(const std::string& path)
{
    if (!storageManager_) return;
    storageManager_->saveAs(path, state_);
}

void WorkspaceSession::commit()
{
    if (storageManager_) {
        try {
            const std::string& path = storageManager_->currentPath();
            if (!path.empty()) storageManager_->save(state_);
        } catch (...) {
            reportException(core::errors::ErrorSeverity::Error, "WorkspaceSession::commit", std::current_exception());
        }
    }

    notifyState();
}

void WorkspaceSession::notifyState()
{
    try {
        if (onStateChanged_) onStateChanged_(state_);
    } catch (...) {
        reportException(core::errors::ErrorSeverity::Error, "WorkspaceSession::notifyState", std::current_exception());
    }
}

void WorkspaceSession::notify()
{
    notifyState();
}

void WorkspaceSession::reportException(core::errors::ErrorSeverity severity,
                                       const char* origin,
                                       std::exception_ptr exception) const
{
    if (errorReporter_) {
        errorReporter_->reportException(severity, origin, exception);
    } else {
        core::errors::reportException(severity, origin, exception);
    }
}

}
