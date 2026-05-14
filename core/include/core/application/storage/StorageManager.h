/**
 * @file core/include/core/application/storage/StorageManager.h
 * @brief Storage manager: repository factory, atomic save/load and registry handling.
 */

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/application/storage/DeletionImpact.h"
#include "core/ports/storage/IRegistry.h"
#include "core/ports/storage/IStorageManager.h"

namespace core::storage {

using IRegistry = core::ports::storage::IRegistry;
using IStorageManager = core::ports::storage::IStorageManager;

/**
 * @brief Persists workspace session state through atomic store callbacks and latest-path registry access.
 */
class StorageManager : public IStorageManager {
public:
    using AtomicStoreSave = IStorageManager::AtomicStoreSave;
    using AtomicStoreLoad = IStorageManager::AtomicStoreLoad;
    using DeletionImpactCallback = IStorageManager::DeletionImpactCallback;

    /** @brief Creates a storage manager with an optional latest-path registry backend. */
    explicit StorageManager(std::shared_ptr<IRegistry> registry = nullptr);

    void setAtomicStoreSave(AtomicStoreSave saveFn) override;
    void setAtomicStoreLoad(AtomicStoreLoad loadFn) override;
    void setDeletionImpactCallback(DeletionImpactCallback cb) override;
    std::optional<std::string> loadLatestPath() const override;
    core::application::workspace::WorkspaceSessionState loadFrom(const std::string& filePath) override;
    void save(const core::application::workspace::WorkspaceSessionState& document) override;
    void saveAs(const std::string& filePath, const core::application::workspace::WorkspaceSessionState& document) override;
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

} // namespace core::storage
