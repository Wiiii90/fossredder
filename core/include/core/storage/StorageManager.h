#pragma once

/**
 * @file core/include/core/storage/StorageManager.h
 * @brief Storage manager: repository factory, atomic save/load and registry handling.
 *
 * Responsible for locating the latest storage file, delegating load/save
 * operations to either atomic callbacks or AppStateManager-backed repositories,
 * and for maintaining a small registry (latest path).
 */

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "core/models/AppState.h"
#include "core/models/DeletionImpact.h"
#include "core/storage/IRegistry.h"
#include "core/storage/IStorageManager.h"

namespace core::storage {

class StorageManager : public IStorageManager {
public:
    using Repositories = IStorageManager::Repositories;
    using RepoFactory = IStorageManager::RepoFactory;
    using AtomicStoreSave = IStorageManager::AtomicStoreSave;
    using AtomicStoreLoad = IStorageManager::AtomicStoreLoad;
    using DeletionImpactCallback = IStorageManager::DeletionImpactCallback;

    explicit StorageManager(std::string appDataDir);
    StorageManager(std::string appDataDir, std::shared_ptr<IRegistry> registry);

    void setRepoFactory(RepoFactory factory) override;
    void setAtomicStoreSave(AtomicStoreSave saveFn) override;
    void setAtomicStoreLoad(AtomicStoreLoad loadFn) override;
    void setDeletionImpactCallback(DeletionImpactCallback cb) override;
    std::optional<std::string> loadLatestPath() const override;
    void setLatestPath(const std::string& filePath) override;
    core::domain::AppState load() override;
    core::domain::AppState loadFrom(const std::string& filePath) override;
    void save(const core::domain::AppState& state) override;
    void saveAs(const std::string& filePath, const core::domain::AppState& state) override;
    void createNew(const std::string& filePath) override;
    const std::string& currentPath() const noexcept override { return currentPath_; }

private:
    std::string appDataDir_;
    std::shared_ptr<IRegistry> registry_;
    std::string currentPath_;
    RepoFactory repoFactory_;
    AtomicStoreSave atomicSave_;
    AtomicStoreLoad atomicLoad_;
    DeletionImpactCallback onDeletionImpact_;

    Repositories reposForCurrent() const;
};

}
