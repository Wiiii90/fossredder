/**
 * @file core/src/managers/StorageManager.cpp
 * @brief Implementation of StorageManager.
 *
 * Implements registry handling (latest path), atomic save/load delegation and
 * repository-based load/save via AppStateManager.
 */

#include "core/pch.h"
#include "core/managers/StorageManager.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>

#include "core/managers/AppStateManager.h"
#include "core/managers/SqliteRegistry.h"

StorageManager::StorageManager(std::string appDataDir)
    : appDataDir_(std::move(appDataDir)) {
    // Use a sqlite-backed registry located in the app data root. This
    // leverages the existing configs table and provides atomic writes.
    try {
        auto dbPath = (std::filesystem::path(appDataDir_) / "fossredder.db").string();
        registry_ = std::make_shared<SqliteRegistry>(dbPath);
    } catch (...) {
        registry_ = nullptr;
    }
}

void StorageManager::setRepoFactory(RepoFactory factory) {
    repoFactory_ = std::move(factory);
}

void StorageManager::setAtomicStoreSave(AtomicStoreSave saveFn) {
    atomicSave_ = std::move(saveFn);
}

void StorageManager::setAtomicStoreLoad(AtomicStoreLoad loadFn) {
    atomicLoad_ = std::move(loadFn);
}

void StorageManager::setDeletionImpactCallback(DeletionImpactCallback cb) {
    onDeletionImpact_ = std::move(cb);
}

std::optional<std::string> StorageManager::loadLatestPath() const {
    if (registry_) return registry_->getLatest();
    return std::nullopt;
}

void StorageManager::setLatestPath(const std::string& filePath) {
    if (registry_) registry_->setLatest(filePath);
}

AppState StorageManager::load() {
    if (currentPath_.empty()) throw std::runtime_error("No file opened");
    return loadFrom(currentPath_);
}

AppState StorageManager::loadFrom(const std::string& filePath) {
    currentPath_ = filePath;
    setLatestPath(filePath);

    if (atomicLoad_) {
        return atomicLoad_(currentPath_);
    }

    auto repos = reposForCurrent();

    AppStateManager::Repositories mgrRepos;
    mgrRepos.actors = repos.actors;
    mgrRepos.properties = repos.properties;
    mgrRepos.contracts = repos.contracts;
    mgrRepos.statements = repos.statements;
    mgrRepos.transactions = repos.transactions;

    AppStateManager mgr(std::move(mgrRepos));
    return mgr.load();
}

void StorageManager::save(const AppState& state) {
    if (currentPath_.empty()) throw std::runtime_error("No file opened");
    saveAs(currentPath_, state);
}

void StorageManager::saveAs(const std::string& filePath, const AppState& state) {
    currentPath_ = filePath;
    setLatestPath(filePath);

    if (atomicSave_) {
        DeletionImpact impact = atomicSave_(currentPath_, state);
        if (onDeletionImpact_ && !impact.empty()) onDeletionImpact_(impact);
        return;
    }

    auto repos = reposForCurrent();

    AppStateManager::Repositories mgrRepos;
    mgrRepos.actors = repos.actors;
    mgrRepos.properties = repos.properties;
    mgrRepos.contracts = repos.contracts;
    mgrRepos.statements = repos.statements;
    mgrRepos.transactions = repos.transactions;

    AppStateManager mgr(std::move(mgrRepos));
    mgr.save(state);
}

void StorageManager::createNew(const std::string& filePath) {
    std::filesystem::path p(filePath);
    auto dir = p.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
    currentPath_ = filePath;
    setLatestPath(filePath);

    if (atomicLoad_) {
        (void)atomicLoad_(currentPath_);
    } else {
        (void)reposForCurrent();
    }
}

void StorageManager::ensureRegistryDir() const {
    // no-op: registry is now sqlite-backed and located in appDataDir_.
}

StorageManager::Repositories StorageManager::reposForCurrent() const {
    if (!repoFactory_) throw std::runtime_error("RepoFactory not configured");
    if (currentPath_.empty()) throw std::runtime_error("No file opened");
    return repoFactory_(currentPath_);
}
