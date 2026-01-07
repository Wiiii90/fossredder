#pragma once

/**
 * @file core/include/core/controllers/AppStateController.h
 * @brief Controller owning the AppState and delegating persistence to IStorageManager.
 */

#include <functional>
#include <memory>
#include <string>
#include <type_traits>

#include "core/controllers/Callbacks.h"
#include "core/managers/IStorageManager.h"

/// UI/domain facing controller that manages the in-memory AppState and uses
/// an IStorageManager for persistence operations.
class AppStateController {
public:
    using StateChanged = std::function<void(const AppState&)>;

    /**
     * @brief Construct with unique ownership of an IStorageManager.
     * @param storageManager Unique pointer to the storage manager implementation.
     */
    explicit AppStateController(std::unique_ptr<IStorageManager> storageManager);

    /** Disable copy, allow move. */
    AppStateController(const AppStateController&) = delete;
    AppStateController& operator=(const AppStateController&) = delete;
    AppStateController(AppStateController&&) noexcept(std::is_nothrow_move_constructible_v<std::unique_ptr<IStorageManager>>) = default;
    AppStateController& operator=(AppStateController&&) noexcept(std::is_nothrow_move_assignable_v<std::unique_ptr<IStorageManager>>) = default;

    /**
     * @brief Register a callback invoked when the AppState changes.
     * @param cb Callback invoked with the new AppState.
     */
    void setStateChangedCallback(StateChanged cb);

    /**
     * @brief Configure repository factory used for repository-backed persistence.
     * @param factory Factory function producing repositories for a DB path.
     */
    void setRepoFactory(IStorageManager::RepoFactory factory);

    /**
     * @brief Configure an atomic save callback used to persist AppState.
     * @param saveFn Callback that performs atomic save and returns DeletionImpact.
     */
    void setAtomicStoreSave(IStorageManager::AtomicStoreSave saveFn);

    /**
     * @brief Configure an atomic load callback used to load AppState.
     * @param loadFn Callback that loads state from a DB path.
     */
    void setAtomicStoreLoad(IStorageManager::AtomicStoreLoad loadFn);

    /**
     * @brief Register a callback reporting IDs deleted during save operations.
     * @param cb Callback receiving a DeletionImpact describing removed IDs.
     */
    void setDeletionImpactCallback(IStorageManager::DeletionImpactCallback cb);

    /**
     * @brief Load the latest configured storage file (if any) and update state.
     */
    void openLatest();

    /**
     * @brief Create a new storage at the given path and reset state.
     * @param path Filesystem path for the new storage.
     */
    void newFile(const std::string& path);

    /**
     * @brief Open storage at given path and load AppState.
     * @param path Filesystem path to open.
     */
    void openFile(const std::string& path);

    /**
     * @brief Save the current AppState to the active storage.
     */
    void saveFile();

    /**
     * @brief Save the current AppState under a new path.
     * @param path New filesystem path to save to.
     */
    void saveFileAs(const std::string& path);

    /**
     * @brief Accessor for the current AppState (const).
     * @return const reference to the current AppState.
     */
    const AppState& state() const noexcept { return state_; }

    /**
     * @brief Accessor for the current AppState (mutable).
     * @return mutable reference to the current AppState.
     */
    AppState& mutableState() noexcept { return state_; }

    /**
     * @brief Return the current storage path managed by the underlying storage manager.
     * @return path string (may be empty if none set).
     */
    const std::string& currentPath() const noexcept { return storageManager_ ? storageManager_->currentPath() : emptyPath_; }

    /**
     * @brief Commit/notify hook — triggers registered state change callbacks.
     *
     * Intended as a single point to emit change notifications after modifications.
     */
    void commit();

private:
    std::unique_ptr<IStorageManager> storageManager_;
    AppState state_;
    StateChanged onStateChanged_;
    std::string emptyPath_;

    /**
     * @brief Internal helper to notify registered listeners of the current state.
     */
    void notify();
};
