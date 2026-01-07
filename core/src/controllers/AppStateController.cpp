/**
 * @file core/src/controllers/AppStateController.cpp
 * @brief Implementation of AppStateController.
 *
 * Implements the runtime behaviour of the AppStateController which manages the
 * in-memory AppState and delegates persistence to an IStorageManager.
 */

#include "core/pch.h"
#include "core/controllers/AppStateController.h"

AppStateController::AppStateController(std::unique_ptr<IStorageManager> storageManager)
    : storageManager_(std::move(storageManager)) {
}

void AppStateController::setStateChangedCallback(StateChanged cb) {
    onStateChanged_ = std::move(cb);
}

void AppStateController::setRepoFactory(IStorageManager::RepoFactory factory) {
    if (storageManager_) storageManager_->setRepoFactory(std::move(factory));
}

void AppStateController::setAtomicStoreSave(IStorageManager::AtomicStoreSave saveFn) {
    if (storageManager_) storageManager_->setAtomicStoreSave(std::move(saveFn));
}

void AppStateController::setAtomicStoreLoad(IStorageManager::AtomicStoreLoad loadFn) {
    if (storageManager_) storageManager_->setAtomicStoreLoad(std::move(loadFn));
}

void AppStateController::setDeletionImpactCallback(IStorageManager::DeletionImpactCallback cb) {
    if (storageManager_) storageManager_->setDeletionImpactCallback(std::move(cb));
}

void AppStateController::openLatest() {
    if (!storageManager_) return;
    if (auto latest = storageManager_->loadLatestPath()) {
        state_ = storageManager_->loadFrom(*latest);
        notify();
    }
}

void AppStateController::newFile(const std::string& path) {
    if (!storageManager_) return;
    storageManager_->createNew(path);
    state_ = AppState{};
    notify();
}

void AppStateController::openFile(const std::string& path) {
    if (!storageManager_) return;
    state_ = storageManager_->loadFrom(path);
    notify();
}

void AppStateController::saveFile() {
    if (!storageManager_) return;
    storageManager_->save(state_);
}

void AppStateController::saveFileAs(const std::string& path) {
    if (!storageManager_) return;
    storageManager_->saveAs(path, state_);
}

void AppStateController::commit() {
    notify();
}

void AppStateController::notify() {
    if (onStateChanged_) onStateChanged_(state_);
}
