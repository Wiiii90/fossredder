#pragma once

/**
 * @file core/include/core/storage/IStorageManager.h
 * @brief Interface for storage manager implementations.
 *
 * Defines the abstract storage manager API used by higher-level controllers.
 */

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "core/models/AppState.h"
#include "core/models/DeletionImpact.h"
#include "core/storage/RepositoryBundle.h"

namespace core::storage {

class IStorageManager {
public:
    using Repositories = RepositoryBundle;

    using RepoFactory = std::function<Repositories(const std::string& dbPath)>;
    using AtomicStoreSave = std::function<DeletionImpact(const std::string& dbPath, const AppState& state)>;
    using AtomicStoreLoad = std::function<AppState(const std::string& dbPath)>;
    using DeletionImpactCallback = std::function<void(const DeletionImpact&)>;

    virtual ~IStorageManager() = default;

    virtual void setRepoFactory(RepoFactory factory) = 0;
    virtual void setAtomicStoreSave(AtomicStoreSave saveFn) = 0;
    virtual void setAtomicStoreLoad(AtomicStoreLoad loadFn) = 0;
    virtual void setDeletionImpactCallback(DeletionImpactCallback cb) = 0;
    virtual std::optional<std::string> loadLatestPath() const = 0;
    virtual void setLatestPath(const std::string& filePath) = 0;
    virtual AppState load() = 0;
    virtual AppState loadFrom(const std::string& filePath) = 0;
    virtual void save(const AppState& state) = 0;
    virtual void saveAs(const std::string& filePath, const AppState& state) = 0;
    virtual void createNew(const std::string& filePath) = 0;
    virtual const std::string& currentPath() const noexcept = 0;
};

}
