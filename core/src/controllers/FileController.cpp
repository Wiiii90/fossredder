#include "core/pch.h"
#include "core/controllers/FileController.h"

#include <stdexcept>

FileController::FileController(FileManager fileManager)
    : fileManager_(std::move(fileManager)) {
}

void FileController::setStateChangedCallback(StateChanged cb) {
    onStateChanged_ = std::move(cb);
}

void FileController::setRepoFactory(FileManager::RepoFactory factory) {
    fileManager_.setRepoFactory(std::move(factory));
}

void FileController::setAtomicStoreSave(FileManager::AtomicStoreSave saveFn) {
    fileManager_.setAtomicStoreSave(std::move(saveFn));
}

void FileController::setAtomicStoreLoad(FileManager::AtomicStoreLoad loadFn) {
    fileManager_.setAtomicStoreLoad(std::move(loadFn));
}

void FileController::setDeletionImpactCallback(DeletionImpactCallback cb) {
    fileManager_.setDeletionImpactCallback(std::move(cb));
}

void FileController::notify() {
    if (onStateChanged_) onStateChanged_(state_);
}

void FileController::openLatest() {
    if (auto latest = fileManager_.loadLatestPath()) {
        state_ = fileManager_.loadFrom(*latest);
        notify();
    }
}

void FileController::newFile(const std::string& path) {
    fileManager_.createNew(path);
    state_ = AppState{};
    notify();
}

void FileController::openFile(const std::string& path) {
    state_ = fileManager_.loadFrom(path);
    notify();
}

void FileController::saveFile() {
    fileManager_.save(state_);
}

void FileController::saveFileAs(const std::string& path) {
    fileManager_.saveAs(path, state_);
}
