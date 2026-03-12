#pragma once

/**
 * @file core/include/core/controllers/AppStateController.h
 * @brief Controller owning the core::domain::AppState and delegating persistence to IStorageManager.
 */

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "core/application/CatalogService.h"
#include "core/errors/IErrorReporter.h"
#include "core/storage/IStorageManager.h"
#include "core/models/DraftStatement.h"
#include "core/models/Transaction.h"

namespace core::application { class WorkspaceSession; }

/// UI/domain facing controller that manages the in-memory core::domain::AppState and uses
/// an IStorageManager for persistence operations.
namespace core::controllers {

class AppStateController {
public:
    using StateChanged = std::function<void(const core::domain::AppState&)>;

    /**
     * @brief Construct with unique ownership of an IStorageManager.
     * @param storageManager Unique pointer to the storage manager implementation.
     */
    explicit AppStateController(std::unique_ptr<core::storage::IStorageManager> storageManager);
    ~AppStateController();

    /** Disable copy, allow move. */
    AppStateController(const AppStateController&) = delete;
    AppStateController& operator=(const AppStateController&) = delete;
    AppStateController(AppStateController&&) noexcept;
    AppStateController& operator=(AppStateController&&) noexcept;

    /**
     * @brief Register a callback invoked when the core::domain::AppState changes.
     * @param cb Callback invoked with the new core::domain::AppState.
     */
    void setStateChangedCallback(StateChanged cb);
    void setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter);

    /**
     * @brief Configure repository factory used for repository-backed persistence.
     * @param factory Factory function producing repositories for a DB path.
     */
    void setRepoFactory(core::storage::IStorageManager::RepoFactory factory);

    /**
     * @brief Configure an atomic save callback used to persist core::domain::AppState.
     * @param saveFn Callback that performs atomic save and returns core::domain::DeletionImpact.
     */
    void setAtomicStoreSave(core::storage::IStorageManager::AtomicStoreSave saveFn);

    /**
     * @brief Configure an atomic load callback used to load core::domain::AppState.
     * @param loadFn Callback that loads state from a DB path.
     */
    void setAtomicStoreLoad(core::storage::IStorageManager::AtomicStoreLoad loadFn);

    /**
     * @brief Register a callback reporting IDs deleted during save operations.
     * @param cb Callback receiving a core::domain::DeletionImpact describing removed IDs.
     */
    void setDeletionImpactCallback(core::storage::IStorageManager::DeletionImpactCallback cb);

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
     * @brief Open storage at given path and load core::domain::AppState.
     * @param path Filesystem path to open.
     */
    void openFile(const std::string& path);

    /**
     * @brief Save the current core::domain::AppState to the active storage.
     */
    void saveFile();

    /**
     * @brief Save the current core::domain::AppState under a new path.
     * @param path New filesystem path to save to.
     */
    void saveFileAs(const std::string& path);

    /**
     * @brief Accessor for the current core::domain::AppState (const).
     * @return const reference to the current core::domain::AppState.
     */
    const core::domain::AppState& state() const noexcept;

    /**
     * @brief Return the current storage path managed by the underlying storage manager.
     * @return path string (may be empty if none set).
     */
    const std::string& currentPath() const noexcept;

    /**
     * @brief core::domain::Actor mutations.
     */
    std::string addActor(const std::string& name, const std::string& type, const std::string& description);
    void updateActor(const std::string& id, const std::string& name, const std::string& type, const std::string& description);
    void deleteActor(const std::string& id);

    /**
     * @brief core::domain::Property mutations.
     */
    std::string addProperty(const std::string& name, const std::string& address, const std::string& description);
    void updateProperty(const std::string& id, const std::string& name, const std::string& address, const std::string& description);
    void deleteProperty(const std::string& id);

    /**
     * @brief core::domain::Contract mutations.
     */
    std::string addContract(const std::string& name, const std::string& type, const std::string& description,
                            const std::vector<std::string>& actorIds, const std::vector<std::string>& propertyIds);
    void updateContract(const std::string& id, const std::string& name, const std::string& type, const std::string& description,
                        const std::vector<std::string>& actorIds, const std::vector<std::string>& propertyIds);
    void deleteContract(const std::string& id);
    std::vector<std::string> contractTypes() const;

    /**
     * @brief core::domain::Statement mutations.
     */
    std::string addStatement(const std::string& name);
    void updateStatement(const std::string& id, const std::string& name);
    void deleteStatement(const std::string& id);

    /**
     * @brief core::domain::Transaction mutations.
     */
    std::string addTransaction(const std::string& name,
                               const std::string& bookingDate,
                               double amount,
                               const std::string& description,
                               const std::string& statementId,
                               core::domain::Transaction::Status status,
                               const std::string& actorId,
                               bool allocatable,
                               const std::vector<std::string>& propertyIds);
    void updateTransaction(const std::string& id,
                           const std::string& name,
                           const std::string& bookingDate,
                           double amount,
                           const std::string& description,
                           const std::string& statementId,
                           core::domain::Transaction::Status status,
                           const std::string& actorId,
                           bool allocatable,
                           const std::vector<std::string>& propertyIds);
    void deleteTransaction(const std::string& id);

    /**
     * @brief core::domain::Analysis mutations.
     */
    std::string addAnalysis(const std::string& name, const std::string& type, const std::string& configJson, const std::string& filterSpec);
    void updateAnalysis(const std::string& id, const std::string& name, const std::string& type, const std::string& configJson, const std::string& filterSpec);
    void deleteAnalysis(const std::string& id);

    /**
     * @brief core::domain::Annual mutations.
     */
    std::string addAnnual(int year);
    void updateAnnual(const std::string& id, int year);
    void deleteAnnual(const std::string& id);

    /**
     * @brief Finalize a temporary draft into persisted statement/transaction/contract domain objects.
     */
    std::string finalizeStatementDraft(const core::domain::DraftStatement& draft);

    /**
     * @brief Persist current state and emit full-state callback.
     */
    void commit();

    /**
     * @brief Notify registered listeners of the current state without persisting.
     *
     * Emits the configured StateChanged callback but does not perform any
     * storage operations. Use when an in-memory update should be reflected in
     * the UI immediately without triggering a save.
     */
    void notifyState();

private:
    core::domain::AppState& mutableState() noexcept;

    core::application::CatalogService catalog_;
    std::unique_ptr<core::application::WorkspaceSession> session_;

};

}
