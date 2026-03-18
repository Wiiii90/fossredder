/**
 * @file core/src/application/WorkspaceSession.cpp
 * @brief Implements the mutable workspace session that owns in-memory app state.
 */

#include "core/application/WorkspaceSession.h"

#include <stdexcept>

namespace core::application {

WorkspaceSession::WorkspaceSession(std::unique_ptr<core::storage::IStorageManager> storageManager)
    : storageManager_(std::move(storageManager))
{
    if (!storageManager_) {
        throw std::invalid_argument("WorkspaceSession requires a storage manager");
    }

    setDeletionImpactCallback({});
}

const std::string& WorkspaceSession::currentPath() const noexcept
{
    return storageManager_->currentPath();
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
    storageManager_->setRepoFactory(std::move(factory));
}

void WorkspaceSession::setAtomicStoreSave(core::storage::IStorageManager::AtomicStoreSave saveFn)
{
    storageManager_->setAtomicStoreSave(std::move(saveFn));
}

void WorkspaceSession::setAtomicStoreLoad(core::storage::IStorageManager::AtomicStoreLoad loadFn)
{
    storageManager_->setAtomicStoreLoad(std::move(loadFn));
}

void WorkspaceSession::setDeletionImpactCallback(core::storage::IStorageManager::DeletionImpactCallback cb)
{
    onDeletionImpact_ = std::move(cb);
    storageManager_->setDeletionImpactCallback([this](const DeletionImpact& impact) {
        try {
            if (onDeletionImpact_) {
                onDeletionImpact_(impact);
            }
        } catch (...) {
            reportException(core::errors::ErrorSeverity::Error,
                            "WorkspaceSession::setDeletionImpactCallback",
                            std::current_exception());
        }
    });
}

void WorkspaceSession::openLatest()
{
    const auto path = storageManager_->loadLatestPath();
    if (!path) {
        return;
    }

    state_ = storageManager_->loadFrom(*path);
    notifyState();
}

void WorkspaceSession::newFile(const std::string& path)
{
    storageManager_->createNew(path);
    state_ = AppState{};
    notifyState();
}

void WorkspaceSession::openFile(const std::string& path)
{
    state_ = storageManager_->loadFrom(path);
    notifyState();
}

void WorkspaceSession::saveFile()
{
    storageManager_->save(state_);
}

void WorkspaceSession::saveFileAs(const std::string& path)
{
    storageManager_->saveAs(path, state_);
}

void WorkspaceSession::commit()
{
    try {
        const std::string& path = storageManager_->currentPath();
        if (!path.empty()) {
            storageManager_->save(state_);
        }
    } catch (...) {
        reportException(core::errors::ErrorSeverity::Error,
                        "WorkspaceSession::commit",
                        std::current_exception());
    }

    notifyState();
}

void WorkspaceSession::notifyState()
{
    try {
        if (onStateChanged_) {
            onStateChanged_(state_);
        }
    } catch (...) {
        reportException(core::errors::ErrorSeverity::Error,
                        "WorkspaceSession::notifyState",
                        std::current_exception());
    }
}

void WorkspaceSession::reportException(core::errors::ErrorSeverity severity,
                                       const char* origin,
                                       std::exception_ptr exception) const
{
    if (!errorReporter_) {
        return;
    }

    errorReporter_->reportException(severity, origin, exception);
}

} // namespace core::application
