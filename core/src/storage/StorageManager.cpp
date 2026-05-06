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
#include <stdexcept>
#include <utility>

#include "core/application/AppStateManager.h"

namespace core::storage {

StorageManager::StorageManager(std::shared_ptr<IRegistry> registry)
    : registry_(std::move(registry)) {
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

void StorageManager::rememberLatestPath(const std::string& filePath) {
    if (registry_) registry_->setLatest(filePath);
}

AppState StorageManager::loadFrom(const std::string& filePath) {
    const std::string previousPath = currentPath_;
    currentPath_ = filePath;

    if (atomicLoad_) {
        try {
            const AppState state = atomicLoad_(currentPath_);
            rememberLatestPath(filePath);
            return state;
        } catch (...) {
            currentPath_ = previousPath;
            throw;
        }
    }

    try {
        core::application::AppStateManager mgr(reposForCurrent());
        const AppState state = mgr.load();
        rememberLatestPath(filePath);
        return state;
    } catch (...) {
        currentPath_ = previousPath;
        throw;
    }
}

void StorageManager::save(const AppState& state) {
    if (currentPath_.empty()) throw std::runtime_error("No file opened");
    saveAs(currentPath_, state);
}

void StorageManager::saveAs(const std::string& filePath, const AppState& state) {
    const std::string previousPath = currentPath_;
    currentPath_ = filePath;

    if (atomicSave_) {
        try {
            DeletionImpact impact = atomicSave_(currentPath_, state);
            rememberLatestPath(filePath);
            if (onDeletionImpact_ && !impact.empty()) onDeletionImpact_(impact);
            return;
        } catch (...) {
            currentPath_ = previousPath;
            throw;
        }
    }

    try {
        core::application::AppStateManager mgr(reposForCurrent());
        mgr.save(state);
        rememberLatestPath(filePath);
    } catch (...) {
        currentPath_ = previousPath;
        throw;
    }
}

void StorageManager::createNew(const std::string& filePath) {
    std::filesystem::path p(filePath);
    auto dir = p.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
    currentPath_ = filePath;

    if (atomicLoad_) {
        (void)atomicLoad_(currentPath_);
    } else {
        (void)reposForCurrent();
    }

    rememberLatestPath(filePath);
}

StorageManager::Repositories StorageManager::reposForCurrent() const {
    if (!repoFactory_) throw std::runtime_error("RepoFactory not configured");
    if (currentPath_.empty()) throw std::runtime_error("No file opened");
    return repoFactory_(currentPath_);
}

}
