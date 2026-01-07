#pragma once

/**
 * @file core/include/core/managers/IStorageManager.h
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

class IActorRepository;
class IPropertyRepository;
class IContractRepository;
class IStatementRepository;
class ITransactionRepository;

class IStorageManager {
public:
    struct Repositories {
        std::shared_ptr<IActorRepository> actors;
        std::shared_ptr<IPropertyRepository> properties;
        std::shared_ptr<IContractRepository> contracts;
        std::shared_ptr<IStatementRepository> statements;
        std::shared_ptr<ITransactionRepository> transactions;
    };

    using RepoFactory = std::function<Repositories(const std::string& dbPath)>;
    using AtomicStoreSave = std::function<DeletionImpact(const std::string& dbPath, const AppState& state)>;
    using AtomicStoreLoad = std::function<AppState(const std::string& dbPath)>;
    using DeletionImpactCallback = std::function<void(const DeletionImpact&)>;

    virtual ~IStorageManager() = default;

    /**
     * @brief Configure repository factory used for non-atomic operations.
     * @param factory Function that creates repository instances for a DB path.
     */
    virtual void setRepoFactory(RepoFactory factory) = 0;

    /**
     * @brief Set an atomic save callback used to persist AppState.
     * @param saveFn Function that performs atomic save and returns DeletionImpact.
     */
    virtual void setAtomicStoreSave(AtomicStoreSave saveFn) = 0;

    /**
     * @brief Set an atomic load callback used to load AppState.
     * @param loadFn Function that loads state from a DB path.
     */
    virtual void setAtomicStoreLoad(AtomicStoreLoad loadFn) = 0;

    /**
     * @brief Register a callback invoked when saves cause deletions.
     * @param cb Callback that receives a DeletionImpact describing removed IDs.
     */
    virtual void setDeletionImpactCallback(DeletionImpactCallback cb) = 0;

    /**
     * @brief Return optional latest storage path from registry.
     * @return optional path string.
     */
    virtual std::optional<std::string> loadLatestPath() const = 0;

    /**
     * @brief Persist the given path as latest in registry.
     * @param filePath Path to persist.
     */
    virtual void setLatestPath(const std::string& filePath) = 0;

    /**
     * @brief Load AppState using configured atomic loader or repo-based manager.
     * @return loaded AppState.
     */
    virtual AppState load() = 0;

    /**
     * @brief Load AppState from specific storage path.
     * @param filePath Path to storage file or DB.
     * @return loaded AppState.
     */
    virtual AppState loadFrom(const std::string& filePath) = 0;

    /**
     * @brief Save AppState to current storage (must be set).
     * @param state AppState to persist.
     */
    virtual void save(const AppState& state) = 0;

    /**
     * @brief Save AppState to a specific path (becomes current).
     * @param filePath Destination path.
     * @param state AppState to persist.
     */
    virtual void saveAs(const std::string& filePath, const AppState& state) = 0;

    /**
     * @brief Create a new storage at the given path (ensures directories).
     * @param filePath Path to new storage file.
     */
    virtual void createNew(const std::string& filePath) = 0;

    /**
     * @brief Current storage path (may be empty).
     * @return path string.
     */
    virtual const std::string& currentPath() const noexcept = 0;
};
