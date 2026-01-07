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

StorageManager::StorageManager(std::string appDataDir)
    : appDataDir_(std::move(appDataDir)) {
    registryPath_ = (std::filesystem::path(appDataDir_) / "files.json").string();
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
    std::ifstream in(registryPath_, std::ios::in);
    if (!in.good()) return std::nullopt;

    std::string s;
    std::getline(in, s, '\0');
    in.close();

    auto pos = s.find("\"latest\"");
    if (pos == std::string::npos) return std::nullopt;
    pos = s.find(':', pos);
    if (pos == std::string::npos) return std::nullopt;
    pos = s.find('"', pos);
    if (pos == std::string::npos) return std::nullopt;
    auto end = s.find('"', pos + 1);
    if (end == std::string::npos) return std::nullopt;
    return s.substr(pos + 1, end - pos - 1);
}

void StorageManager::setLatestPath(const std::string& filePath) {
    ensureRegistryDir();
    std::ofstream out(registryPath_, std::ios::trunc);
    if (!out.good()) return;
    out << "{\"latest\":\"";
    for (char c : filePath) {
        if (c == '\\') out << "\\\\";
        else if (c == '"') out << "\\\"";
        else out << c;
    }
    out << "\"}";
    out.close();
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
    std::filesystem::path p(registryPath_);
    auto dir = p.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
}

StorageManager::Repositories StorageManager::reposForCurrent() const {
    if (!repoFactory_) throw std::runtime_error("RepoFactory not configured");
    if (currentPath_.empty()) throw std::runtime_error("No file opened");
    return repoFactory_(currentPath_);
}
