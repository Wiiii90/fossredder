#pragma once

#include "core/errors/IErrorReporter.h"
#include "core/models/AppState.h"
#include "core/models/DeletionImpact.h"
#include "core/storage/IStorageManager.h"

#include <functional>
#include <memory>
#include <string>

namespace core::application {

class WorkspaceSession {
public:
    using StateChanged = std::function<void(const core::domain::AppState&)>;

    explicit WorkspaceSession(std::unique_ptr<core::storage::IStorageManager> storageManager);

    const core::domain::AppState& state() const noexcept { return state_; }
    core::domain::AppState& mutableState() noexcept { return state_; }
    const std::string& currentPath() const noexcept;

    void setStateChangedCallback(StateChanged cb);
    void setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter);
    void setRepoFactory(core::storage::IStorageManager::RepoFactory factory);
    void setAtomicStoreSave(core::storage::IStorageManager::AtomicStoreSave saveFn);
    void setAtomicStoreLoad(core::storage::IStorageManager::AtomicStoreLoad loadFn);
    void setDeletionImpactCallback(core::storage::IStorageManager::DeletionImpactCallback cb);

    void openLatest();
    void newFile(const std::string& path);
    void openFile(const std::string& path);
    void saveFile();
    void saveFileAs(const std::string& path);
    void commit();
    void notifyState();

private:
    void reportException(core::errors::ErrorSeverity severity,
                         const char* origin,
                         std::exception_ptr exception) const;

    std::unique_ptr<core::storage::IStorageManager> storageManager_;
    core::domain::AppState state_;
    StateChanged onStateChanged_;
    std::shared_ptr<core::errors::IErrorReporter> errorReporter_;
    core::storage::IStorageManager::DeletionImpactCallback onDeletionImpact_;
    inline static const std::string emptyPath_;
};

} // namespace core::application