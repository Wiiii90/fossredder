/**
 * @file core/src/controllers/AppStateController.cpp
 * @brief Implementation of AppStateController.
 *
 * Implements the runtime behaviour of the AppStateController which manages the
 * in-memory AppState and delegates persistence to an IStorageManager.
 */

#include "core/pch.h"
#include "core/controllers/AppStateController.h"
#include <unordered_set>

AppStateController::AppStateController(std::unique_ptr<IStorageManager> storageManager)
    : storageManager_(std::move(storageManager)) {
    // forward storage deletion impacts to registered callback if storageManager supports it
    if (storageManager_) {
        storageManager_->setDeletionImpactCallback([this](const DeletionImpact& impact){
            try {
                if (onDeletionImpact_) onDeletionImpact_(impact);
            } catch(...) {}
        });
    }
}

void AppStateController::notifyState() {
    try {
        if (onStateChanged_) onStateChanged_(state_);
    } catch (...) {}
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
    onDeletionImpact_ = std::move(cb);
    if (storageManager_) storageManager_->setDeletionImpactCallback([this](const DeletionImpact& impact){
        try { if (onDeletionImpact_) onDeletionImpact_(impact); } catch(...) {}
    });
}

void AppStateController::openLatest() {
    if (!storageManager_) return;
    if (auto latest = storageManager_->loadLatestPath()) {
        try {
            fprintf(stderr, "AppStateController::openLatest: loading latest='%s'\n", latest->c_str());
        } catch(...) {}
        state_ = storageManager_->loadFrom(*latest);
        try {
            fprintf(stderr, "AppStateController::openLatest: loaded state - actors=%zu props=%zu contracts=%zu statements=%zu transactions=%zu\n",
                    state_.actors.size(), state_.properties.size(), state_.contracts.size(), state_.statements.size(), state_.transactions.size());
        } catch(...) {}
        notify();
    }
}

void AppStateController::newFile(const std::string& path) {
    if (!storageManager_) return;
    try {
        fprintf(stderr, "AppStateController::newFile: creating new file '%s' (will reset state)\n", path.c_str());
    } catch(...) {}
    storageManager_->createNew(path);
    state_ = AppState{};
    try {
        fprintf(stderr, "AppStateController::newFile: state reset -> actors=%zu props=%zu contracts=%zu statements=%zu transactions=%zu\n",
                state_.actors.size(), state_.properties.size(), state_.contracts.size(), state_.statements.size(), state_.transactions.size());
    } catch(...) {}
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
    // Persist immediately to avoid relying solely on application close
    // (helps ensure UI-created changes are saved reliably).
    if (storageManager_) {
        try {
            const std::string& path = storageManager_->currentPath();
            if (!path.empty()) {
                storageManager_->save(state_);
            }
        } catch (const std::exception& ex) {
            // swallow persistence errors but log to stderr for diagnostics
            fprintf(stderr, "AppStateController::commit: save failed: %s\n", ex.what());
        } catch (...) {
            fprintf(stderr, "AppStateController::commit: save failed: unknown error\n");
        }
    }

    // After persistence, prefer incremental transaction notifications to avoid
    // triggering a full UI state reload which causes visible flicker during
    // inline edits. If there are dirty transaction ids, invoke the granular
    // callback; otherwise fall back to the full state changed callback.
    try {
        // If storage manager reported deletions synchronously during save, forward
        // them to the registered deletion impact callback. (StorageManager may
        // already invoke its own callback; this ensures AppStateController exposes
        // the same information to higher layers.)
        // Note: onDeletionImpact_ is invoked via storageManager wrapper as well.
        if (!dirtyTransactionIds_.empty() && onTransactionsChanged_) {
            std::vector<std::string> ids;
            ids.reserve(dirtyTransactionIds_.size());
            for (const auto& s : dirtyTransactionIds_) ids.push_back(s);
            onTransactionsChanged_(ids);
        } else {
            if (onStateChanged_) onStateChanged_(state_);
        }
    } catch (...) {}
    dirtyTransactionIds_.clear();
}

void AppStateController::markTransactionDirty(const std::string& txId) {
    if (txId.empty()) return;
    dirtyTransactionIds_.insert(txId);
}

void AppStateController::setTransactionsChangedCallback(TransactionsChanged cb) {
    onTransactionsChanged_ = std::move(cb);
}

void AppStateController::notify() {
    if (onStateChanged_) onStateChanged_(state_);
}
