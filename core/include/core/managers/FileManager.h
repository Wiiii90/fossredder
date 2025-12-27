#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "core/models/AppState.h"
#include "core/models/DeletionImpact.h"

class IActorRepository;
class IPropertyRepository;
class IContractRepository;
class IStatementRepository;
class ITransactionRepository;

class FileManager {
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

    explicit FileManager(std::string appDataDir);

    void setRepoFactory(RepoFactory factory);
    void setAtomicStoreSave(AtomicStoreSave saveFn);
    void setAtomicStoreLoad(AtomicStoreLoad loadFn);
    void setDeletionImpactCallback(DeletionImpactCallback cb);

    std::optional<std::string> loadLatestPath() const;
    void setLatestPath(const std::string& filePath);

    AppState load();
    AppState loadFrom(const std::string& filePath);

    void save(const AppState& state);
    void saveAs(const std::string& filePath, const AppState& state);

    void createNew(const std::string& filePath);

    const std::string& currentPath() const noexcept { return currentPath_; }

private:
    std::string appDataDir_;
    std::string registryPath_;
    std::string currentPath_;

    RepoFactory repoFactory_;
    AtomicStoreSave atomicSave_;
    AtomicStoreLoad atomicLoad_;
    DeletionImpactCallback onDeletionImpact_;

    void ensureRegistryDir() const;
    Repositories reposForCurrent() const;
};
