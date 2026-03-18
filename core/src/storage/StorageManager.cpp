/**
 * @file core/src/storage/StorageManager.cpp
 * @brief Implementation of StorageManager.
 *
 * Implements registry handling (latest path), atomic save/load delegation and
 * repository-based load/save via AppStateManager.
 */

#include "core/pch.h"
#include "core/storage/StorageManager.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>

#include "core/application/AppStateManager.h"
#include "core/storage/SqliteRegistry.h"

namespace core::storage {

namespace {

std::shared_ptr<IRegistry> createDefaultRegistry(const std::string& appDataDir) {
    try {
        const auto dbPath = (std::filesystem::path(appDataDir) / "fossredder.db").string();
        return std::make_shared<SqliteRegistry>(dbPath);
    } catch (...) {
        return nullptr;
    }
}

} // namespace

StorageManager::StorageManager(std::string appDataDir)
    : StorageManager(std::move(appDataDir), nullptr) {}

StorageManager::StorageManager(std::string appDataDir, std::shared_ptr<IRegistry> registry)
    : appDataDir_(std::move(appDataDir))
    , registry_(std::move(registry)) {
    if (!registry_) {
        registry_ = createDefaultRegistry(appDataDir_);
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

    core::application::AppStateManager mgr(reposForCurrent());
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

    core::application::AppStateManager mgr(reposForCurrent());
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

StorageManager::Repositories StorageManager::reposForCurrent() const {
    if (!repoFactory_) throw std::runtime_error("RepoFactory not configured");
    if (currentPath_.empty()) throw std::runtime_error("No file opened");
    return repoFactory_(currentPath_);
}

}
