#pragma once

/**
 * @file core/include/core/managers/StorageManager.h
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
#include "core/managers/IStorageManager.h"

class IActorRepository;
class IPropertyRepository;
class IContractRepository;
class IStatementRepository;
class ITransactionRepository;

class StorageManager : public IStorageManager {
public:
    using Repositories = IStorageManager::Repositories;

    using RepoFactory = IStorageManager::RepoFactory;
    using AtomicStoreSave = IStorageManager::AtomicStoreSave;
    using AtomicStoreLoad = IStorageManager::AtomicStoreLoad;
    using DeletionImpactCallback = IStorageManager::DeletionImpactCallback;

    /**
     * @brief Construct a StorageManager bound to given application data directory.
     * @param appDataDir Path where registry (files.json) is stored.
     */
    explicit StorageManager(std::string appDataDir);

    /** Implement IStorageManager */
    /**
     * @brief Configure repository factory used for non-atomic operations.
     * @param factory Function producing repositories for a DB path.
     */
    void setRepoFactory(RepoFactory factory) override;

    /**
     * @brief Configure an atomic save callback used to persist AppState.
     * @param saveFn Callback performing atomic save and returning DeletionImpact.
     */
    void setAtomicStoreSave(AtomicStoreSave saveFn) override;

    /**
     * @brief Configure an atomic load callback used to load AppState.
     * @param loadFn Callback that loads AppState from a DB path.
     */
    void setAtomicStoreLoad(AtomicStoreLoad loadFn) override;

    /**
     * @brief Register a callback invoked when saves remove rows (deletion impact).
     * @param cb Callback receiving DeletionImpact.
     */
    void setDeletionImpactCallback(DeletionImpactCallback cb) override;

    /**
     * @brief Return optional latest storage path from registry.
     * @return optional path string (empty if not present).
     */
    std::optional<std::string> loadLatestPath() const override;

    /**
     * @brief Persist the given path as latest in registry.
     * @param filePath Filesystem path to persist.
     */
    void setLatestPath(const std::string& filePath) override;

    /**
     * @brief Load AppState using configured atomic loader or repo-based manager.
     * @return Loaded AppState.
     */
    AppState load() override;

    /**
     * @brief Load AppState from a specific storage path.
     * @param filePath Path to load from.
     * @return Loaded AppState.
     */
    AppState loadFrom(const std::string& filePath) override;

    /**
     * @brief Save AppState to the current storage (must be set).
     * @param state AppState to persist.
     */
    void save(const AppState& state) override;

    /**
     * @brief Save AppState to a specific path and mark it current.
     * @param filePath Destination path.
     * @param state AppState to persist.
     */
    void saveAs(const std::string& filePath, const AppState& state) override;

    /**
     * @brief Create a new storage at the given path (ensures directories exist).
     * @param filePath Path to new storage file.
     */
    void createNew(const std::string& filePath) override;

    /**
     * @brief Return the current storage path (may be empty).
     * @return current path string.
     */
    const std::string& currentPath() const noexcept override { return currentPath_; }

private:
    std::string appDataDir_;
    std::string registryPath_;
    std::string currentPath_;

    RepoFactory repoFactory_;
    AtomicStoreSave atomicSave_;
    AtomicStoreLoad atomicLoad_;
    DeletionImpactCallback onDeletionImpact_;

    /**
     * @brief Ensure the directory for the registry file exists.
     */
    void ensureRegistryDir() const;

    /**
     * @brief Create repository instances for the current storage path.
     * @return Repositories object constructed via the configured factory.
     * @throws std::runtime_error if repoFactory_ is not configured or no file opened.
     */
    Repositories reposForCurrent() const;
};
