#include "core/pch.h"
#include "core/managers/FileManager.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>

#include "core/managers/AppStateManager.h"

FileManager::FileManager(std::string appDataDir)
    : appDataDir_(std::move(appDataDir)) {
    registryPath_ = (std::filesystem::path(appDataDir_) / "files.json").string();
}

void FileManager::setRepoFactory(RepoFactory factory) {
    repoFactory_ = std::move(factory);
}

void FileManager::setAtomicStoreSave(AtomicStoreSave saveFn) {
    atomicSave_ = std::move(saveFn);
}

void FileManager::setAtomicStoreLoad(AtomicStoreLoad loadFn) {
    atomicLoad_ = std::move(loadFn);
}

void FileManager::setDeletionImpactCallback(DeletionImpactCallback cb) {
    onDeletionImpact_ = std::move(cb);
}

void FileManager::ensureRegistryDir() const {
    std::filesystem::path p(registryPath_);
    auto dir = p.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
}

FileManager::Repositories FileManager::reposForCurrent() const {
    if (!repoFactory_) throw std::runtime_error("RepoFactory not configured");
    if (currentPath_.empty()) throw std::runtime_error("No file opened");
    return repoFactory_(currentPath_);
}

std::optional<std::string> FileManager::loadLatestPath() const {
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

void FileManager::setLatestPath(const std::string& filePath) {
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

AppState FileManager::load() {
    if (currentPath_.empty()) throw std::runtime_error("No file opened");
    return loadFrom(currentPath_);
}

AppState FileManager::loadFrom(const std::string& filePath) {
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

void FileManager::save(const AppState& state) {
    if (currentPath_.empty()) throw std::runtime_error("No file opened");
    saveAs(currentPath_, state);
}

void FileManager::saveAs(const std::string& filePath, const AppState& state) {
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

void FileManager::createNew(const std::string& filePath) {
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
