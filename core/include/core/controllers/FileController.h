#pragma once

#include <functional>
#include <memory>
#include <string>

#include "core/controllers/DeletionImpactCallback.h"
#include "core/managers/FileManager.h"

class FileController {
public:
    using StateChanged = std::function<void(const AppState&)>;

    explicit FileController(FileManager fileManager);

    void setStateChangedCallback(StateChanged cb);
    void setRepoFactory(FileManager::RepoFactory factory);
    void setAtomicStoreSave(FileManager::AtomicStoreSave saveFn);
    void setAtomicStoreLoad(FileManager::AtomicStoreLoad loadFn);
    void setDeletionImpactCallback(DeletionImpactCallback cb);

    void openLatest();
    void newFile(const std::string& path);
    void openFile(const std::string& path);
    void saveFile();
    void saveFileAs(const std::string& path);

    const AppState& state() const noexcept { return state_; }
    AppState& mutableState() noexcept { return state_; }
    const std::string& currentPath() const noexcept { return fileManager_.currentPath(); }

private:
    FileManager fileManager_;
    AppState state_;
    StateChanged onStateChanged_;

    void notify();
};
