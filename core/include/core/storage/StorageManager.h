#pragma once

/**
 * @file core/include/core/storage/StorageManager.h
 * @brief Storage manager: repository factory, atomic save/load and registry handling.
 *
 * Responsible for locating the latest storage file, delegating load/save
 * operations to either atomic callbacks or AppStateManager-backed repositories,
 * and for updating an injected registry abstraction (latest path).
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
    using AtomicStoreSave = IStorageManager::AtomicStoreSave;
    using AtomicStoreLoad = IStorageManager::AtomicStoreLoad;
    using DeletionImpactCallback = IStorageManager::DeletionImpactCallback;

    explicit StorageManager(std::shared_ptr<IRegistry> registry = nullptr);

    void setAtomicStoreSave(AtomicStoreSave saveFn) override;
    void setAtomicStoreLoad(AtomicStoreLoad loadFn) override;
    void setDeletionImpactCallback(DeletionImpactCallback cb) override;
    std::optional<std::string> loadLatestPath() const override;
    core::domain::AppState loadFrom(const std::string& filePath) override;
    void save(const core::domain::AppState& state) override;
    void saveAs(const std::string& filePath, const core::domain::AppState& state) override;
    void createNew(const std::string& filePath) override;
    const std::string& currentPath() const noexcept override { return currentPath_; }

private:
    std::shared_ptr<IRegistry> registry_;
    std::string currentPath_;
    AtomicStoreSave atomicSave_;
    AtomicStoreLoad atomicLoad_;
    DeletionImpactCallback onDeletionImpact_;
    void rememberLatestPath(const std::string& filePath);
};

}
