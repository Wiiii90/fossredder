#pragma once

/**
 * @file core/include/core/storage/IStorageManager.h
 * @brief Interface for storage manager implementations.
 *
 * Defines the abstract storage manager API used by higher-level application services.
 */

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "core/models/AppState.h"
#include "core/models/DeletionImpact.h"

namespace core::storage {

class IStorageManager {
public:
    using AtomicStoreSave = std::function<core::domain::DeletionImpact(const std::string& dbPath, const core::domain::AppState& state)>;
    using AtomicStoreLoad = std::function<core::domain::AppState(const std::string& dbPath)>;
    using DeletionImpactCallback = std::function<void(const core::domain::DeletionImpact&)>;

    virtual ~IStorageManager() = default;

    virtual void setAtomicStoreSave(AtomicStoreSave saveFn) = 0;
    virtual void setAtomicStoreLoad(AtomicStoreLoad loadFn) = 0;
    virtual void setDeletionImpactCallback(DeletionImpactCallback cb) = 0;
    virtual std::optional<std::string> loadLatestPath() const = 0;
    virtual core::domain::AppState loadFrom(const std::string& filePath) = 0;
    virtual void save(const core::domain::AppState& state) = 0;
    virtual void saveAs(const std::string& filePath, const core::domain::AppState& state) = 0;
    virtual void createNew(const std::string& filePath) = 0;
    virtual const std::string& currentPath() const noexcept = 0;
};

}
